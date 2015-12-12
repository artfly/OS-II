#include "client.hpp"


Client::Client(int sock) :  state(CONNECT_REMOTE), remote_state(WAIT),
													 chunk_to_read(0), total(0), requested_header(0) {
	client_conn = new Connection(sock);
	remote_conn = NULL;
	cache = Cache::get_instance();
	entry = NULL;
	request_header = NULL;
	response_header = NULL;
}

Client::~Client() {
	std::cout << "Close sock" << std::endl;
	delete client_conn;
	std::cout << "Close sock" << std::endl;
	if (remote_conn)
		delete remote_conn;
	// if (entry && response_header->get_code() != response_header->OK_CODE) {
	// 	std::cout << "Close sock" << std::endl;
	// 	delete entry;
	// }
	std::cout << "Close sock" << std::endl;
	if (request_header) {
		delete request_header;
	}
	if (response_header) {
		delete response_header;
	}
	std::cout << "Close sock" << std::endl;
}

int Client::work(short events, int sock) {
	if (client_conn->get_sock() == sock) {
		return client_work(events);
	}
	else if (remote_conn && remote_conn->get_sock() == sock) {
		return remote_work(events);
	}
	else {
		std::cerr << "Exception!";
		exit(1);
	}
}

int Client::client_work(short events) {
	switch (state) {
		case CONNECT_REMOTE:
		 	if (events & POLLIN) {						//!!!!
		 		connect_server();
		 	}
		 	else if (remote_conn) {
		 		if (!request_header->check_header()) {
		 			std::string msg = request_header->get_error_msg();
					client_conn->send_data(msg.c_str(), msg.size());
					state = EXIT_CLIENT;
		 		}
		 		else {
		 			entry = cache->get_entry(request_header->get_url());
		 			state = READ_CACHE;
		 			if (!entry) {
		 				entry = new CacheEntry();
		 				remote_state = READ_HEADER;
		 				return remote_conn->get_sock();
		 			}
		 		}
		 	}
		break;

		case READ_CACHE:
		 	if (events & POLLOUT) {
		 		const char * chunk = entry->get_data(chunk_to_read);
		 		int chunk_len = entry->get_length(chunk_to_read);
		 		if (!chunk && entry->is_finished()) {
		 			state = EXIT_CLIENT;
		 		}
		 		else if (chunk) {
		 			int sent = client_conn->send_data(chunk, chunk_len);
		 			if (sent >= 0) {
		 				chunk_to_read++;								//?
		 			}
		 			// if (chunk_to_read != 0)
		 			// 	total += sent;
		 			// std::cout << "Sent total " << total << " to " << client_conn->get_sock() << std::endl;
		 		}
		 	}
		break;

		case EXIT_CLIENT:
			std::cout << "Client bye-bye!" << std::endl;
	}
	return 0;
}


int Client::remote_work(short events) {
	switch (remote_state) {
		case WAIT:
		break;
		case READ_HEADER:
			if (events & POLLOUT) {
				if (requested_header <= 0) {
					requested_header = remote_conn->send_data(client_conn->get_buffer(),
																	 client_conn->get_length());
				}
				// if (remote_conn->send_data(client_conn->get_buffer(), client_conn->get_length()) < 0)
				// 	remote_state = EXIT_REMOTE;
			}
			if (events & POLLIN) {
				if (read_remote_header()) {
					remote_state = READ_CONTENT;
				}
			} 
		break;

		case READ_CONTENT:
			if (events & POLLIN) {
				std::cout << "Remote event POLLIN" << std::endl;
				if (!read_remote_data()) {
					std::cout << "SOCKET CLOSED" << std::endl;
					remote_state = EXIT_REMOTE;
				}
			}
			if (events & POLLOUT) {
				std::cout << "total : " << total << std::endl;
				std::cout << "required : " << response_header->get_length() << std::endl;
				if (response_header->get_length() >= 0 && total >= response_header->get_length()) {
					entry->set_finished();
					if (response_header->get_code() == response_header->OK_CODE)
						cache->put_entry(request_header->get_url(), entry);
					remote_state = EXIT_REMOTE;
				}
				else if (response_header->get_length() < 0) {
					char c;
					if (recv(remote_conn->get_sock(), &c, 1, MSG_PEEK | MSG_DONTWAIT) <= 0) {
						std::cout << "END CHECK" << std::endl;
						if (errno != EAGAIN) {
							entry->set_finished();
							if (response_header->get_code() == response_header->OK_CODE)
								cache->put_entry(request_header->get_url(), entry);
							remote_state = EXIT_REMOTE;
						}
					}
				}
			}
		break;

		case EXIT_REMOTE:
			std::cout << "Server bye-bye!" << std::endl;
	}
	return 0;
}

bool Client::read_remote_header() {
	if (remote_conn->recv_data() <= 0) {
		std::cout << "WTF? Where is all the data." << std::endl;						//?!
		return false;
	}
	remote_conn->get_buffer()[7] = '0';													//?
	std::cout << client_conn->get_sock() << remote_conn->get_buffer() << std::endl;
	response_header = new ResponseHeader(remote_conn->get_buffer());
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	total += remote_conn->get_length() - response_header->get_header_len();
	return true;
}

bool Client::read_remote_data() {
	if (remote_conn->recv_data() < 0) {										//?
		entry->set_finished();
		if (response_header->get_code() == response_header->OK_CODE)
			cache->put_entry(request_header->get_url(), entry);
		return false;
	}
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	total += remote_conn->get_length();
	return true;
}

void Client::connect_server() {
	if (client_conn->recv_data() <= 0)											//?
		return;
	request_header = new RequestHeader(client_conn->get_buffer());
	if (!request_header->check_header())
		return;
	std::cout << "REQUEST : " << request_header->get_data() << std::endl;
	int s = 0;
	sockaddr_in host_addr;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return;
	}
	host_addr.sin_family = AF_INET;
	if ((host_addr.sin_addr.s_addr = inet_addr(request_header->get_host().c_str())) == INADDR_NONE) {
		struct hostent *hp;
		if ((hp = gethostbyname(request_header->get_host().c_str())) == NULL) {
			std::cerr << "error : get host by name error" << std::endl;
			std::cerr << "host was " << request_header->get_host() << std::endl; 
			close(s);
			s = -1;
			return;
		}
		host_addr.sin_family = hp->h_addrtype;
		memcpy(&(host_addr.sin_addr), hp->h_addr, hp->h_length);
	}
	host_addr.sin_port = htons(request_header->get_port());
	if (connect(s, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0) {
		close(s);
		return;
	}
	remote_conn = new Connection(s);
	std::cout << "Сonnection established." << std::endl;
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