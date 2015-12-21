#ifndef PROXY_HPP
#define PROXY_HPP

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <map>
#include "client.hpp"

class Proxy {
 public:
	Proxy(int port);
	~Proxy();
	void run();
 	void say_hi();
 	void remove_from_poll(int sock); 
 	void add_to_poll(int socket, Client * client, int mode);
 private:
 	const static int BACKLOG = 50;
 	const static int MAX_CLIENTS = 256;

 	struct pollfd poll_list[MAX_CLIENTS];
 	int proxy_socket;
 	int poll_size;
 	std::map<int, Client*> clients;

	int init_socket(int port);
 	void add_proxy();
 	void add_client();
 	void remove_dead();
};

#endif