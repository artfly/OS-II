#include "client.hpp"
#include "proxy.hpp"


Client::Client(int sock, Proxy * proxy) : state(CONNECT_REMOTE), remote_state(WAIT),
									chunk_to_read(0), total(0), proxy(proxy) {
	client_conn = new Connection(sock);
	remote_conn = NULL;
	cache = Cache::get_instance();
	entry = NULL;
	request_header = NULL;
	response_header = NULL;
}

Client::~Client() {
	std::cout << "DEBUG : destructor for sock " << client_conn->get_sock() << std::endl;
	if (client_conn)
		delete client_conn;
	if (remote_conn)
		delete remote_conn;
	if (request_header) {
		delete request_header;
	}
	if (entry) {
		entry->remove_reader();
	}
	// if (response_header->get_code() != ResponseHeader::OK_CODE) {
	// 	std::cout << "DEBUG : code : " << response_header->get_code() << std::endl;
	// 	delete entry;
	// }
	if (response_header) {
		delete response_header;
	}
	// std::cout << "DEBUG : end of dtor" << std::endl;
}

void Client::work(short events, int sock) {
	if (client_conn->get_sock() == sock) {
		client_work(events);
	}
	else if (remote_conn && remote_conn->get_sock() == sock) {
		remote_work(events);
	}
	else {
		std::cerr << "Exception!";
		exit(1);
	}
}

void Client::client_work(short events) {
	std::cout << "DEBUG client state : " << state << " events : " << events << std::endl;
	switch (state) {
		case CONNECT_REMOTE: {
			connect_server();
			break;
		}
		case READ_CACHE: {
			read_cache();
			break;
		}
		case SEND_ERROR: {
			send_error();
			break;
		}
		case EXIT_CLIENT: {
			std::cout << "Client bye-bye!" << std::endl;
		}
	}
}

void Client::connect_server() {
	if (client_conn->recv_data() <= 0) {	
		state = EXIT_CLIENT;									
		return;
	}
	request_header = new RequestHeader(client_conn->get_buffer());
	if (!request_header->check_header()) {
		proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
		state = SEND_ERROR;
		return;
	}
	
	entry = cache->get_entry(request_header->get_url());
	if (entry) {
		proxy->attach_to_remote(this, request_header->get_url());
		proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
		entry->add_reader();
		state = READ_CACHE;
		return;
	}

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
	
	if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr)) == 0 || errno == EINPROGRESS) {
		remote_conn = new Connection(s);
		proxy->remove_from_poll(client_conn->get_sock());
		proxy->add_to_poll(remote_conn->get_sock(), this, POLLOUT);
		remote_state = REQUEST_HEADER;
	}
	else {
		std::cout << "error : connection error" << std::endl;
		close(s);
		state = EXIT_CLIENT;
		return;
	}
}

void Client::read_cache() {
	const char * chunk = entry->get_data(chunk_to_read);
	int chunk_len = entry->get_length(chunk_to_read);
	bool finished = entry->is_finished();
	if (!chunk && finished) {
		state = EXIT_CLIENT;
		return;
	}

	if (chunk) {
		int sent = client_conn->send_data(chunk, chunk_len);
		if (sent < 0) {
			if (remote_conn)
				proxy->remove_from_poll(client_conn->get_sock());
			proxy->detach_from_remote(this, request_header->get_url());
			// delete client_conn;
			if (!remote_conn)
				state = EXIT_CLIENT;
			return;
		}
		chunk_to_read++;
	}

	if (!finished && !chunk)
		proxy->remove_from_poll(client_conn->get_sock());
}

void Client::send_error() {
	// std::cout << "DEBUG : send error" <<  std::endl;
	std::string msg = request_header->get_error_msg();
	client_conn->send_data(msg.c_str(), msg.size());
	state = EXIT_CLIENT;
	// std::cout << "DEBUG : send error success" <<  std::endl;
}


void Client::remote_work(short events) {
	std::cout << "DEBUG server state : " << remote_state << " events : " << events << std::endl;
	// std::cout << "DEBUG : remote_state : " << remote_state << std::endl;
	// std::cout << "DEBUG : remote events : " << events << std::endl;
	switch (remote_state) {
		case WAIT: {
			break;
		}
		case REQUEST_HEADER: {
			if (!send_request_header())
				break;
			remote_state = READ_HEADER;
			proxy->add_to_poll(remote_conn->get_sock(), this, POLLIN);
			break;
		}
		case READ_HEADER: {
			if (!read_remote_header())
				break;
			state = READ_CACHE;
			remote_state = READ_CONTENT;
			proxy->add_to_poll(remote_conn->get_sock(), this, POLLIN|POLLOUT);
			break;
		}
		case READ_CONTENT: {
			if (!cache->get_entry(entry->get_url()) && find(clients.begin(), clients.end(), this) == clients.end()) {
				proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
				std::cout << "DEBUG : state is EXIT_CLIENT now" << std::endl;
				state = EXIT_CLIENT;
			}
			if (events & POLLIN) {
				notify_clients();
				if (!read_remote_data()) {
					std::cout << "READ REMOTE FAILED" << std::endl;
					break;
				}
				proxy->add_to_poll(remote_conn->get_sock(), this, POLLIN|POLLOUT);
			}
			if (events & POLLOUT) {
				if (response_header->get_length() >= 0 && total >= response_header->get_length()) {
					entry->set_finished();
					proxy->remove_from_poll(remote_conn->get_sock());								//TEST, ADD TO 1 IF OK
					notify_clients();
					remote_state = EXIT_REMOTE;
					if (find(clients.begin(), clients.end(), this) == clients.end()) {
						proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
						state = EXIT_CLIENT;
					}
				}
				else {
					proxy->add_to_poll(remote_conn->get_sock(), this, POLLIN);
				}
			}
			break;
		}
		case EXIT_REMOTE: {
			if (state == EXIT_CLIENT)
				proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
			break;
		}
	}
}

bool Client::send_request_header() {
	int error = 0;
	socklen_t len = sizeof(error);
	getsockopt(remote_conn->get_sock(), SOL_SOCKET, SO_ERROR, &error, &len);	
	if (error) {
		std::cerr << "error : server connect error" << std::endl;
		state = EXIT_CLIENT;
		return false;
	}
	remote_conn->send_data(client_conn->get_buffer(), client_conn->get_length());
	return true;
}

bool Client::read_remote_header() {
	if (remote_conn->recv_data() <= 0) {
		state = EXIT_CLIENT;
		return false;
	}

	entry = cache->get_entry(request_header->get_url());
	//someone else added connection. start cache read.
	if (entry) {
		proxy->remove_from_poll(remote_conn->get_sock());
		delete remote_conn;
		state = READ_CACHE;
		proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
		entry->add_reader();
		return false;
	}

	response_header = new ResponseHeader(remote_conn->get_buffer());
	entry = new CacheEntry(request_header->get_url());		
	std::cout << response_header->get_code() << std::endl; 
	if (response_header->get_code() == response_header->OK_CODE)
		cache->put_entry(entry, response_header->get_length());			
	// std::cout << "DEBUG : adding new data to entry" << std::endl;					
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	clients.push_back(this);
	notify_clients();
	// std::cout << "DEBUG : added new data to entry" << std::endl;					
	total += remote_conn->get_length() - response_header->get_header_len();
	return true;
}

bool Client::read_remote_data() {
	int recv_bytes;
	if ((recv_bytes = remote_conn->recv_data()) <= 0) {			
		std::cout << "DEBUG : recv_bytes : " << recv_bytes << std::endl;						
		entry->set_finished();
		remote_state = EXIT_REMOTE;
		proxy->remove_from_poll(remote_conn->get_sock());
		if (find(clients.begin(), clients.end(), this) == clients.end()) {
			proxy->add_to_poll(client_conn->get_sock(), this, POLLOUT);
			state = EXIT_CLIENT;
		}
		return false;
	}
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	total += remote_conn->get_length();
	return true;
}

bool Client::alive() const {
	if (state == EXIT_CLIENT && response_header && response_header->get_code() != ResponseHeader::OK_CODE)
		delete entry;
	return state != EXIT_CLIENT;
}

bool Client::attach_to_remote(Client * client, std::string url) {
	if (remote_conn && !url.compare(request_header->get_url())) {
		clients.push_back(client);
		return true;
	}
	return false;
}

bool Client::detach_from_remote(Client * client, std::string url) {
	if (remote_conn && !url.compare(request_header->get_url())) {
		clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
		return true;
	}
	return false;
}

void Client::notify_clients() {
  	for (size_t i = 0; i < clients.size(); i++) {
  		proxy->add_to_poll(clients[i]->get_client_sock(), clients[i], POLLOUT);
  	}
}

int Client::get_client_sock() const {
	if (client_conn)
		return client_conn->get_sock();
	return -1;
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