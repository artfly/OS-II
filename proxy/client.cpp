#include "client.hpp"
#include "proxy.hpp"


Client::Client(int sock, Proxy * proxy) : state(CONNECT_REMOTE), remote_state(WAIT),
									chunk_to_read(0), total(0), proxy(proxy) {
	proxy->say_hi();
	client_conn = new Connection(sock);
	remote_conn = NULL;
	cache = Cache::get_instance();
	entry = NULL;
	request_header = NULL;
	response_header = NULL;
}

Client::~Client() {
	std::cout << "DEBUG : destructor for sock " << client_conn->get_sock() << std::endl;
	delete client_conn;
	if (remote_conn)
		delete remote_conn;
	if (request_header) {
		delete request_header;
	}
	if (response_header) {
		delete response_header;
	}
	if (entry) {
		entry->remove_reader();
	}
}

void Client::work(short events, int sock) {
	if (client_conn->get_sock() == sock) {
		// std::cout << "DEBUG : client work :" << std::endl;
		// if (request_header)
		// 	std::cout << request_header->get_url() << std::endl;
		client_work(events);
	}
	else if (remote_conn && remote_conn->get_sock() == sock) {
		// std::cout << "DEBUG : server work" << std::endl;
		remote_work(events);
		// std::cout << "DEBUG : states after : " << std::endl;
		// std::cout << state << " " << remote_state << std::endl;
		// std::cout << "DEBUG : url " << request_header->get_url() << std::endl;
	}
	else {
		std::cerr << "Exception!";
		exit(1);
	}
}

void Client::client_work(short events) {
	switch (state) {
		case CONNECT_REMOTE: {
			connect_server();
			break;
		}
		case READ_CACHE: {
			const char * chunk = entry->get_data(chunk_to_read);
	 		int chunk_len = entry->get_length(chunk_to_read);
	 		if (!chunk && entry->is_finished()) {
	 			// std::cout << "DEBUG : client finished. url : " << request_header->get_url() << std::endl;
	 			state = EXIT_CLIENT;
	 		}
	 		else if (chunk) {
	 			int sent = client_conn->send_data(chunk, chunk_len);
	 			if (sent >= 0) {
	 				chunk_to_read++;								
	 			}
	 			else {
	 				state = EXIT_CLIENT;
	 			}
		 	}
		 	if (remote_state != EXIT_REMOTE && state != EXIT_CLIENT && remote_conn)
		 		proxy->remove_from_poll(client_conn->get_sock());
			break;
		}
		case SEND_ERROR: {
			std::string msg = request_header->get_error_msg();
			client_conn->send_data(msg.c_str(), msg.size());
			state = EXIT_CLIENT;
			break;
		}
		case EXIT_CLIENT: {
			std::cout << "Client bye-bye!" << std::endl;
		}
	}
}


	// switch (state) {
	// 	case CONNECT_REMOTE:
	// 	 	if (events & POLLIN) {						
	// 	 		connect_server();
	// 	 	}
	// 	 	else if (remote_conn) {
	// 	 		if (!request_header->check_header()) {
	// 	 			std::string msg = request_header->get_error_msg();
	// 				client_conn->send_data(msg.c_str(), msg.size());
	// 				state = EXIT_CLIENT;
	// 	 		}
	// 	 		else {
	// 	 			if (remote_conn->is_connecting())
	// 	 				break;
	// 	 			entry = cache->get_entry(request_header->get_url());
	// 	 			state = READ_CACHE;
	// 	 			if (!entry) {
	// 	 				entry = new CacheEntry();
	// 	 				remote_state = READ_HEADER;
	// 	 				return remote_conn->get_sock();
	// 	 			}
	// 	 		}
	// 	 	}
	// 	break;

	// 	case READ_CACHE:
	// 	 	if (events & POLLOUT) {
	// 	 		const char * chunk = entry->get_data(chunk_to_read);
	// 	 		int chunk_len = entry->get_length(chunk_to_read);
	// 	 		if (!chunk && entry->is_finished()) {
	// 	 			state = EXIT_CLIENT;
	// 	 		}
	// 	 		else if (chunk) {
	// 	 			int sent = client_conn->send_data(chunk, chunk_len);
	// 	 			if (sent >= 0) {
	// 	 				chunk_to_read++;								
	// 	 			}
	// 	 		}
	// 	 	}
	// 	break;
	// 	case SEND_ERROR:
	// 		if (events & POLLOUT) {
	// 			std::string msg = request_header->get_error_msg();
	// 			client_conn->send_data(msg.c_str(), msg.size());
	// 			state = EXIT_CLIENT;
	// 		}
	// 	case EXIT_CLIENT:
	// 		std::cout << "Client bye-bye!" << std::endl;
	// }
	// return 0;
// }


void Client::remote_work(short events) {
	switch (remote_state) {
		case WAIT: {
			break;
		}
		case REQUEST_HEADER: {
			//check for errors during connect
			int error = 0;
			socklen_t len = sizeof(error);
			getsockopt(remote_conn->get_sock(), SOL_SOCKET, SO_ERROR, &error, &len);	
			if (error) {
				std::cerr << "error : server connect error" << std::endl;
				state = EXIT_CLIENT;
			}
			//send client request and wait for response
			remote_conn->send_data(client_conn->get_buffer(), client_conn->get_length());
			proxy->add_to_poll(remote_conn->get_sock(), this, POLLIN);
			// std::cout << "DEBUG : sent request header" << std::endl;
			remote_state = READ_HEADER;
			break;
		}
		case READ_HEADER: {
			read_remote_header();
			state = READ_CACHE;
			remote_state = READ_CONTENT;
			proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
			proxy->add_to_poll(remote_conn->get_sock(), this, POLLIN|POLLOUT);
			break;
		}
		case READ_CONTENT: {
			if (events & POLLIN ) {
				if (!read_remote_data()) {
					remote_state = EXIT_REMOTE;
				}
			}
			if (events & POLLOUT) {
				if (response_header->get_length() >= 0 && total >= response_header->get_length()) {
					entry->set_finished();
					remote_state = EXIT_REMOTE;
				}
			}
			proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
			break;
		}
		case EXIT_REMOTE: {
			// proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
			// std::cout << "Server bye-bye!" << std::endl;
		}

	}

	// switch (remote_state) {
	// 	case WAIT:
	// 	break;
	// 	case READ_HEADER:
	// 		if (events & POLLOUT) {
	// 			if (requested_header <= 0) {
	// 				int error = 0;
	// 				socklen_t len = sizeof(error);
	// 				getsockopt(remote_conn->get_sock(), SOL_SOCKET, SO_ERROR, &error, &len);
	// 				if (error != 0) {
	// 					std::cerr << "error : server connect error" << std::endl;
	// 					state = EXIT_CLIENT;
	// 				}
	// 				remote_conn->set_connecting(false);
	// 				requested_header = remote_conn->send_data(client_conn->get_buffer(),
	// 																 client_conn->get_length());
	// 			}
	// 		}
	// 		if (events & POLLIN) {
	// 			if (read_remote_header()) {
	// 				remote_state = READ_CONTENT;
	// 			}
	// 		} 
	// 	break;

	// 	case READ_CONTENT:
	// 		if (events & POLLIN) {
	// 			if (!read_remote_data()) {
	// 				remote_state = EXIT_REMOTE;
	// 			}
	// 		}
	// 		if (events & POLLOUT) {
	// 			if (response_header->get_length() >= 0 && total >= response_header->get_length()) {
	// 				entry->set_finished();
	// 				if (response_header->get_code() == response_header->OK_CODE)
	// 					cache->put_entry(request_header->get_url(), entry);
	// 				remote_state = EXIT_REMOTE;
	// 			}
	// 		}
	// 	break;
	// 	case EXIT_REMOTE:
	// 	break;
	// 		// std::cout << "Server bye-bye!" << std::endl;
	// }
	// return 0;
}

void Client::read_remote_header() {
	if (remote_conn->recv_data() <= 0) {
		state = EXIT_CLIENT;
		return;
	}
	// remote_conn->get_buffer()[7] = '0';	
	response_header = new ResponseHeader(remote_conn->get_buffer());
	//create entry and decide if local
	// std::cout << "DEBUG : creating new entry" << std::endl;
	entry = new CacheEntry();		
	if (response_header->get_code() == response_header->OK_CODE)
		cache->put_entry(request_header->get_url(), entry);			
	// std::cout << "DEBUG : adding new data to entry" << std::endl;					
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	// std::cout << "DEBUG : added new data to entry" << std::endl;					
	total += remote_conn->get_length() - response_header->get_header_len();
}

bool Client::read_remote_data() {
	int recv_bytes;
	if ((recv_bytes = remote_conn->recv_data()) <= 0) {									
		entry->set_finished();
		return false;
	}
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	total += remote_conn->get_length();
	return true;
}

void Client::connect_server() {
	//smth went wrong, abort
	if (client_conn->recv_data() <= 0) {	
		state = EXIT_CLIENT;									
		return;
	}
	request_header = new RequestHeader(client_conn->get_buffer());
	//invalid header
	if (!request_header->check_header()) {
		proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
		state = SEND_ERROR;
		return;
	}
	//check for cache entry
	// std::cout << "DEBUG : getting entry" << std::endl;
	entry = cache->get_entry(request_header->get_url());
	if (entry) {
		proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
		state = READ_CACHE;
		return;
	}

	// std::cout << "DEBUG : connecting server" << std::endl;
	//connect to server
	int s = 0;
	sockaddr_in host_addr;
	if ((s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
		return;
	}
	host_addr.sin_family = AF_INET;
	if ((host_addr.sin_addr.s_addr = inet_addr(request_header->get_host().c_str())) == INADDR_NONE) {
		struct hostent *hp;
		// std::cout << "Getting host by name" << std::endl;
		// std::cout << request_header->get_host() << std::endl;
		if ((hp = gethostbyname(request_header->get_host().c_str())) == NULL) {
			std::cerr << "error : get host by name error" << std::endl; 
			close(s);
			s = -1;
			return;
		}
		host_addr.sin_family = hp->h_addrtype;
		memcpy(&(host_addr.sin_addr), hp->h_addr, hp->h_length);
	}
	host_addr.sin_port = htons(request_header->get_port());
	// std::cout << "DEBUG : starting connect" << std::endl;
	if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr)) == 0) {
		std::cout << "Сonnection established." << std::endl;
		remote_conn = new Connection(s);
		proxy->remove_from_poll(client_conn->get_sock());
		proxy->add_to_poll(remote_conn->get_sock(), this, POLLOUT);
		remote_state = REQUEST_HEADER;
	}
	else if (errno == EINPROGRESS) {
		// std::cout << "Сonnection would be established." << std::endl;
		remote_conn = new Connection(s);
		proxy->add_to_poll(remote_conn->get_sock(), this, POLLOUT);
		proxy->remove_from_poll(client_conn->get_sock());
		remote_state = REQUEST_HEADER;
		// std::cout << "Сonnection would be established." << std::endl;
	}
	else {
		std::cout << "error : connection error" << std::endl;
		close(s);
		return;
	}
}

bool Client::alive() const {
	return state != EXIT_CLIENT;
}

int Client::get_sock(int sock) const {
	if (remote_conn && sock == remote_conn->get_sock()) {
		return client_conn->get_sock();
	}
	else if (remote_conn && sock == client_conn->get_sock()) {
		return remote_conn->get_sock();
	}
	return -1;
}