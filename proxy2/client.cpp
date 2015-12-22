#include "client.hpp"


Client::Client(int sock) :  poll_size(2), state(CONNECT_REMOTE), remote_state(WAIT),
													 chunk_to_read(0), total(0), requested_header(0) {
	client_conn = new Connection(sock);
	remote_conn = NULL;
	cache = Cache::get_instance();
	request_header = NULL;
	response_header = NULL;
	create_poll();
}

void Client::create_poll() {
	memset(poll_list, -1 , sizeof(poll_list));
	add_to_poll(client_conn->get_sock(), POLLIN);
	poll_size = 2;
}

Client::~Client() {
	// std::cout << "DEBUG : destructor for " << client_conn->get_sock() <<std::endl;
	delete client_conn;
	// std::cout << "DEBUG : dtor start" << std::endl;
	//danger!!!
	if (entry && remote_conn && response_header->get_code() != ResponseHeader::OK_CODE) {
		delete entry;
	}
	// std::cout << "DEBUG : dtor ok?" << std::endl;
	if (remote_conn)
		delete remote_conn;
	if (request_header) {
		delete request_header;
	}
	if (response_header) {
		delete response_header;
	}
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
				work(element.revents, element.fd);
			}
		}
		if (!alive()) {
			return;
		}
	}
}

void Client::add_to_poll(int sock, int mode) {
	//change mode
	for (int i = 0; i < 2; i++) {
		if (poll_list[i].fd == sock) {
			poll_list[i].events = mode;
			return;
		}
	}

	//add new
	int index;
	if (poll_list[0].fd <= 0)
		index = 0;
	else
		index = 1;
	poll_list[index].fd = sock;
	poll_list[index].events = mode;
}

void Client::remove_from_poll(int sock) {
	for (int i = 0; i < 2; i++) {
		if (poll_list[i].fd == sock) {
			poll_list[i].fd = -1;
			return;
		}
	}
}

void Client::work(short events, int sock) {
	if (client_conn->get_sock() == sock) {
		// std::cout << "DEBUG : client work :" << client_conn->get_sock() << std::endl;
		client_work(events);
		// if (request_header)
		// 	std::cout << request_header->get_url() << std::endl;
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
	// std::cout << "DEBUG : states " << state << " " << remote_state << " sock : " << client_conn->get_sock() <<  std::endl;
	// std::cout << "DEBUG : poll : " << poll_list[0].fd << " " << poll_list[0].events << std::endl; 
	// std::cout << "DEBUG : poll : " << poll_list[1].fd << " " << poll_list[1].events << std::endl; 
	switch (state) {
		case CONNECT_REMOTE: {
			connect_server();
			break;
		}
		case READ_CACHE: {
			if (!remote_conn) {
				// std::cout << "DEBUG : getting emutex, sock : " << client_conn->get_sock() << std::endl;
			}
			pthread_mutex_t * emutex = entry->get_entry_mutex();
			pthread_mutex_lock(emutex);
			if (!remote_conn) {
				// std::cout << "DEBUG : aquired mutex, sock : " << client_conn->get_sock() << std::endl;
			}
			const char * chunk = entry->get_data(chunk_to_read);
	 		int chunk_len = entry->get_length(chunk_to_read);
	 		bool finished = entry->is_finished();

	 		//client without remote waits for signal
	 		if (!remote_conn && !chunk && !finished) {
	 			std::cout << "cond start" << std::endl;
	 			pthread_cond_t * econd = entry->get_entry_cond();
	 			pthread_cond_wait(econd, emutex);
	 			chunk = entry->get_data(chunk_to_read);
		 		chunk_len = entry->get_length(chunk_to_read);
		 		finished = entry->is_finished();
		 		std::cout << "cond end" << std::endl;
	 		}

	 		if (!remote_conn) {
				// std::cout << "DEBUG : chunk and finished state, sock : " << client_conn->get_sock() << std::endl;
				// std::cout << chunk_len << " " << finished << std::endl;
			}

	 		//finish
	 		if (!chunk && finished) {
	 			pthread_mutex_t * rmutex = entry->get_readers_mutex();
	 			// std::cout << "DEBUG : let me guess. Sigfault?" << std::endl;
	 			entry->remove_reader();
	 			std::cout << "DEBUG : zero readers? " << !entry->is_used() << std::endl;
	 			std::cout << "DEBUG : url : " << request_header->get_url() << std::endl;
	 			pthread_mutex_unlock(rmutex);
	 			pthread_mutex_unlock(emutex);
	 			// std::cout << "DEBUG : client finished. url : " << request_header->get_url() << std::endl;
	 			state = EXIT_CLIENT;
	 			break;
	 		}
		 	pthread_mutex_unlock(emutex);
	 		//normal work
	 		if (chunk) {
	 			int sent = client_conn->send_data(chunk, chunk_len);
	 			if (sent >= 0) {
	 				chunk_to_read++;								
	 			}
	 			else {
	 				std::cout << "OMG OMG OMG OMG OMG" << std::endl;
	 				state = EXIT_CLIENT;
	 			}
	 		}
	 		//client without remote waits for signal
		 	if (remote_state != EXIT_REMOTE && state != EXIT_CLIENT && remote_conn) {
		 		// std::cout << "DEBUG : client removes himself from poll " << std::endl;
		 		remove_from_poll(client_conn->get_sock());
		 	}
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


void Client::remote_work(short events) {
	// std::cout << "DEBUG : states " << state << " " << remote_state << std::endl;
	// std::cout << "DEBUG : revents on remote " << events << std::endl;
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
			add_to_poll(remote_conn->get_sock(), POLLIN);
			// std::cout << "DEBUG : sent request header" << std::endl;
			remote_state = READ_HEADER;
			break;
		}
		case READ_HEADER: {
			if (!read_remote_header())
				break;
			state = READ_CACHE;
			remote_state = READ_CONTENT;
			add_to_poll(client_conn->get_sock(), POLLOUT);
			add_to_poll(remote_conn->get_sock(), POLLIN|POLLOUT);
			break;
		}
		case READ_CONTENT: {
			if (events & POLLIN) {
				// std::cout << "DEBUG : remote get data" << std::endl;
				if (!read_remote_data()) {
					break;
				}
				add_to_poll(client_conn->get_sock(), POLLOUT);
				add_to_poll(remote_conn->get_sock(), POLLIN|POLLOUT);			//?
			}
			if (events & POLLOUT) {
				if (response_header->get_length() >= 0 && total >= response_header->get_length()) {
					pthread_mutex_t * emutex = entry->get_entry_mutex();
					pthread_cond_t * econd = entry->get_entry_cond();
					pthread_mutex_lock(emutex);
					entry->set_finished();
					pthread_cond_signal(econd);
					pthread_mutex_unlock(emutex);
					remove_from_poll(remote_conn->get_sock());								//TEST, ADD TO 1 IF OK
					add_to_poll(client_conn->get_sock(), POLLOUT);
					remote_state = EXIT_REMOTE;
				}
				else {															//? for no length and early try
					add_to_poll(remote_conn->get_sock(), POLLIN);
				}
			}
			break;
		}
		case EXIT_REMOTE: {
			remove_from_poll(remote_conn->get_sock());								//TEST, ADD TO 1 IF OK
			add_to_poll(client_conn->get_sock(), POLLOUT);
			// std::cout << "Server bye-bye!" << std::endl;
			break;
		}

	}
}

bool Client::read_remote_header() {
	if (remote_conn->recv_data() <= 0) {
		state = EXIT_CLIENT;
		return false;
	}
	// remote_conn->get_buffer()[7] = '0';	
	response_header = new ResponseHeader(remote_conn->get_buffer());
	//create entry and decide if local
	// std::cout << "DEBUG : creating new entry" << std::endl;
	pthread_mutex_t * cmutex = cache->get_cache_mutex();
	pthread_mutex_lock(cmutex);
	entry = cache->get_entry(request_header->get_url());
	//someone else added connection. start cache read.
	if (entry) {
		remove_from_poll(remote_conn->get_sock());
		delete remote_conn;
		state = READ_CACHE;
		add_to_poll(client_conn->get_sock(), POLLOUT);
		pthread_mutex_t * rmutex = entry->get_readers_mutex();
		pthread_mutex_lock(rmutex);
		entry->add_reader();
		pthread_mutex_unlock(rmutex);
		pthread_mutex_unlock(cmutex);
		return false;
	}
	//new entry
	entry = new CacheEntry();		
	if (response_header->get_code() == response_header->OK_CODE)
		cache->put_entry(request_header->get_url(), entry);			
	// std::cout << "DEBUG : adding new data to entry" << std::endl;					
	pthread_mutex_unlock(cmutex);
	pthread_mutex_t * emutex = entry->get_entry_mutex();
	pthread_cond_t * econd = entry->get_entry_cond();
	pthread_mutex_lock(emutex);
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	pthread_cond_signal(econd);
	pthread_mutex_unlock(emutex);
	// std::cout << "DEBUG : added new data to entry" << std::endl;					
	total += remote_conn->get_length() - response_header->get_header_len();
	return true;
}

bool Client::read_remote_data() {
	int recv_bytes;
	pthread_mutex_t * emutex = entry->get_entry_mutex();
	pthread_cond_t * econd = entry->get_entry_cond();
	pthread_mutex_lock(emutex);
	if ((recv_bytes = remote_conn->recv_data()) <= 0) {
		entry->set_finished();
		pthread_cond_signal(econd);
		pthread_mutex_unlock(emutex);
		remove_from_poll(remote_conn->get_sock());								//TEST, ADD TO 1 IF OK
		add_to_poll(client_conn->get_sock(), POLLOUT);
		remote_state = EXIT_REMOTE;
		return false;
	}
	entry->append_data(remote_conn->get_buffer(), remote_conn->get_length());
	pthread_cond_signal(econd);
	pthread_mutex_unlock(emutex);
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
		add_to_poll(client_conn->get_sock(), POLLOUT);
		state = SEND_ERROR;
		return;
	}
	//check for cache entry
	// std::cout << "DEBUG : getting entry" << std::endl;
	pthread_mutex_t * cmutex = cache->get_cache_mutex();
	// std::cout << "DEBUG : getting cmutex" << std::endl;
	pthread_mutex_lock(cmutex);
	// std::cout << "DEBUG : got cmutex. wow." << std::endl;
	entry = cache->get_entry(request_header->get_url());
	if (entry) {
		pthread_mutex_t * rmutex = entry->get_readers_mutex();
		pthread_mutex_lock(rmutex);
		entry->add_reader();
		pthread_mutex_unlock(rmutex);
		add_to_poll(client_conn->get_sock(), POLLOUT);
		state = READ_CACHE;
		pthread_mutex_unlock(cmutex);
		return;
	}
	pthread_mutex_unlock(cmutex);

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
		remove_from_poll(client_conn->get_sock());
		add_to_poll(remote_conn->get_sock(), POLLOUT);
		remote_state = REQUEST_HEADER;
	}
	else if (errno == EINPROGRESS) {
		// std::cout << "Сonnection would be established." << std::endl;
		remote_conn = new Connection(s);
		add_to_poll(remote_conn->get_sock(), POLLOUT);
		remove_from_poll(client_conn->get_sock());
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