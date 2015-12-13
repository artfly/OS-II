#include "client.hpp"


Client::Client(int sock) :  poll_size(2), state(CONNECT_REMOTE), remote_state(WAIT),
													 chunk_to_read(0), total(0), requested_header(0) {
	client_conn = new Connection(sock);
	remote_conn = NULL;
	cache = Cache::get_instance();
	// entry = new CacheEntry;
	request_header = NULL;
	response_header = NULL;
	create_poll();
}

void Client::create_poll() {
	memset(poll_list, -1 , sizeof(poll_list));
	poll_list[0].fd = client_conn->get_sock();
	poll_list[0].events = POLLIN | POLLOUT;
	poll_size = 2;
}

Client::~Client() {
	// std::cout << "Close sock" << std::endl;
	delete client_conn;
	// std::cout << "Close sock" << std::endl;
	if (remote_conn)
		delete remote_conn;
	// if (entry && response_header->get_code() != response_header->OK_CODE) {
	// 	std::cout << "Close sock" << std::endl;
	// 	delete entry;
	// }
	// std::cout << "Close sock" << std::endl;
	if (request_header) {
		std::cout << "Got all data from " << request_header->get_url() << std::endl;
		delete request_header;
	}
	if (response_header) {
		delete response_header;
	}
	// std::cout << "Close sock" << std::endl;
}

void Client::run() {
	while(1) {
		if (poll(poll_list, poll_size, -1) < 0) {
			std::cerr << "eror : poll error" << std::endl;
			continue;
		}
		for (int i = 0; i < poll_size; i++) {
			pollfd element = poll_list[i];
			if (element.revents) {
				// printf("event from : %d\n", element.fd);
				add_to_poll(work(element.revents, element.fd));
			}
			if (!alive()) {
				return;
			}
		}
	}
	//what next?
}

void Client::add_to_poll(int remote_sock) {
	if (remote_sock) {
		poll_list[1].fd = remote_sock;
		poll_list[1].events = POLLIN | POLLOUT;
		std::cout << "added server : " << remote_sock << std::endl;
	}
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
		 	if (events & POLLIN) {						
		 		Mutex * mutex = Mutex::get_instance();
		 		pthread_mutex_t * smutex = mutex->get_socket_mutex();
				pthread_mutex_lock(smutex);
		 		connect_server();
				pthread_mutex_unlock(smutex);
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
		 		char * chunk = (char *)malloc(entry->get_length(chunk_to_read));
		 		entry->get_data(chunk_to_read, chunk);
		 		int chunk_len = entry->get_length(chunk_to_read);
		 		if (!entry->get_length(chunk_to_read) && entry->is_finished()) {
		 			state = EXIT_CLIENT;
		 		}
		 		else if (entry->get_length(chunk_to_read)) {
		 			// std::cout << "SENDING CHUNK : " << std::endl;
		 			// std::cout << "CHUNK NUM" << chunk_to_read << std::endl;
		 			// std::cout << "CLIENT IS " << client_conn->get_sock() << std::endl;
		 			// std::cout << "REQUEST WAS" << request_header->get_url() << std::endl;
		 			// write(1, chunk, chunk_len);
		 			// std::cout << "\n";
		 			int sent = client_conn->send_data(chunk, chunk_len);
		 			if (sent >= 0) {
		 				chunk_to_read++;								//?
		 				// std::cout << "CHUNK SENT" << std::endl;
		 				free(chunk);
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
				// std::cout << "Remote event POLLIN" << std::endl;
				if (!read_remote_data()) {
					// std::cout << "SOCKET CLOSED" << std::endl;
					remote_state = EXIT_REMOTE;
				}
			}
			if (events & POLLOUT) {
				// std::cout << "total : " << total << std::endl;
				// std::cout << "required : " << response_header->get_length() << std::endl;
				if (response_header->get_length() >= 0 && total >= response_header->get_length()) {
					entry->set_finished();
					if (response_header->get_code() == response_header->OK_CODE) {
						pthread_mutex_t * cmutex = Mutex::get_instance()->get_cache_mutex();
						pthread_mutex_lock(cmutex);
						cache->put_entry(request_header->get_url(), entry);
						pthread_mutex_unlock(cmutex);
					}
					remote_state = EXIT_REMOTE;
				}
				else if (response_header->get_length() < 0) {
					char c;
					// std::cout << "END CHECK" << std::endl;
					if (recv(remote_conn->get_sock(), &c, 1, MSG_PEEK) <= 0) {
						// std::cout << "TIME TO DIE?" << std::endl;
						if (errno != EAGAIN) {
							entry->set_finished();
							// std::cout << "YEAH, IT'S TIME" << std::endl;
							if (response_header->get_code() == response_header->OK_CODE) {
								// std::cout << "HEADER CODE " << std::endl;
								pthread_mutex_t * cmutex = Mutex::get_instance()->get_cache_mutex();
								pthread_mutex_lock(cmutex);
								Cache::get_instance()->put_entry(request_header->get_url(), entry);
								// std::cout << "ADDED ENTRY " << std::endl;
								pthread_mutex_unlock(cmutex);
							}
							remote_state = EXIT_REMOTE;
						}
					}
					// std::cout << "NOT TIME TO DIE" << std::endl;
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
		// std::cout << "WTF? Where is all the data." << std::endl;						//?!
		return false;
	}
	remote_conn->get_buffer()[7] = '0';													//?
	// std::cout << client_conn->get_sock() << remote_conn->get_buffer() << std::endl;
	response_header = new ResponseHeader(remote_conn->get_buffer());
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	total += remote_conn->get_length() - response_header->get_header_len();
	return true;
}

bool Client::read_remote_data() {
	if (remote_conn->recv_data() < 0) {										//?
		entry->set_finished();
		if (response_header->get_code() == response_header->OK_CODE) {
			pthread_mutex_t * cmutex = Mutex::get_instance()->get_cache_mutex();
			pthread_mutex_lock(cmutex);
			cache->put_entry(request_header->get_url(), entry);
			pthread_mutex_unlock(cmutex);
		}
		return false;
	}
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	total += remote_conn->get_length();
	return true;
}

void Client::connect_server() {
	if (client_conn->recv_data() <= 0) {											//?
		// std::cout << "No data from client during server connect" << std::endl;
		// state = EXIT_CLIENT;
		return;
	}
	request_header = new RequestHeader(client_conn->get_buffer());
	if (!request_header->check_header()) {
		std::cout << "Wrong header : " << request_header->get_data() << std::endl;
		return;
	}
	// std::cout << "REQUEST : " << request_header->get_data() << std::endl;
	int s = 0;
	sockaddr_in host_addr;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return;
	}
	host_addr.sin_family = AF_INET;
	if ((host_addr.sin_addr.s_addr = inet_addr(request_header->get_host().c_str())) == INADDR_NONE) {
		struct hostent *hp;
		std::cout << "Resource : " << request_header->get_url() << std::endl;
		std::cout << "Getting host by name" << std::endl;
		std::cout << request_header->get_host() << std::endl;
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
	std::cout << "Сonnection established, client " << client_conn->get_sock() << std::endl;
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