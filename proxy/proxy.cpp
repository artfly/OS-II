#include "proxy.hpp"

Proxy::Proxy(int port) : proxy_socket(-1), poll_size(0) {
	proxy_socket = init_socket(port);
	add_proxy();
}

Proxy::~Proxy() {
	close(proxy_socket);
	clients.clear();
	Cache::reset_instance();
}

int Proxy::init_socket(int port) {
	int s = 0;
	int enable = 1;
	struct sockaddr_in addr;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		std::cerr << "error : could not create socket" << std::endl;
		exit(1);	
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		close(s);
		std::cerr << "error : could not set proxy socket options" << std::endl;
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if ((bind( s, (struct sockaddr *) &addr, sizeof(addr))) < 0) {
		close(s);
		std::cerr << "error : could not bind proxy socket" << std::endl;
		exit(1);
	}
	if ((listen(s, BACKLOG)) < 0) {
		close(s);
		std::cerr << "error : could not init listen" << std::endl;
		exit(1);
	}
	return s;
}

void Proxy::add_proxy() {
	memset(poll_list, 0 , sizeof(poll_list));
	poll_list[0].fd = proxy_socket;
	poll_list[0].events = POLLIN;
	poll_size = 1;
}



void Proxy::run() {
	std::cout << "Proxy is up and running" << std::endl;
	while(1) {
		// std::cout << "DEBUG : waiting events " << std::endl;
		if (poll(poll_list, poll_size, -1) < 0) {
			std::cerr << "eror : poll error" << std::endl;
			continue;
		}
		// std::cout << "DEBUG : poll state :" << std::endl;
		for (int i = 1; i < poll_size; i++) {
			pollfd element = poll_list[i];
			// std::cout << element.fd << " ";
			if (element.fd < 0)
				continue;
			Client * cur_client = clients[element.fd];
			if (element.revents) {
				cur_client->work(element.revents, element.fd);
			}
		}
		// std::cout << "\n";
		if (poll_list[0].revents && POLLIN) {
			add_client();
		}
		remove_dead();	
	}
}

void Proxy::add_client() {
	struct sockaddr_in client_addr;
	int addr_size = sizeof((struct sockaddr *) &client_addr);
	int client_sock;
	if ((client_sock = accept(proxy_socket, (struct sockaddr*)&client_addr, (socklen_t *)&addr_size)) < 0) {
		std::cerr << "error : could not add client, errno : " << errno << std::endl;
		return;
	}
	std::cout << "connection from client" << client_sock << std::endl;
	add_to_poll(client_sock, new Client(client_sock, this), POLLIN);
}

void Proxy::add_to_poll(int sock, Client * client, int mode) {
	//change mode
	for (int i = 1; i < poll_size; i++) {
		if (poll_list[i].fd == sock) {
			poll_list[i].events = mode;	
			return;
		}
	}

	//add new
	for (int i = 1; i < MAX_CLIENTS; i++) {
		if (poll_list[i].fd <= 0) {
			poll_list[i].fd = sock;
			poll_list[i].events = mode;
			if (i >= poll_size) poll_size++;
			clients[sock] = client;
			return;
		}
	}
}

void Proxy::remove_from_poll(int sock) {
	for (int i = 1; i < poll_size; i++) {
		if (poll_list[i].fd == sock) {
			poll_list[i].fd = -1;
			return;
		}
	}
}

void Proxy::remove_dead() {
	std::map<int, Client *>::iterator it;
	std::map<int, Client *>::iterator it_remote;
	for (int i = 1; i < poll_size; i++) {
		it = clients.find(poll_list[i].fd);
		if (it != clients.end() && !(it->second->alive())) {
			it_remote = clients.find(it->second->get_sock(poll_list[i].fd));
			if (it_remote != clients.end())
				clients.erase(it_remote);
			delete it->second;
			clients.erase(it);
			poll_list[i].fd = -1;
		}
		else if (it == clients.end()) {
			poll_list[i].fd = -1;
		}
	}
}

void Proxy::say_hi() {
	// std::cout << "Hi!" << std::endl;
}