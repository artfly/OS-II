#ifndef PROXY_HPP
#define PROXY_HPP

#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <map>
#include "client.hpp"
#include "mutex.hpp"
#include <pthread.h>

class Proxy {
 public:
	Proxy(int port);
	~Proxy();
	void run();
	static void * client_body(void * param);
 private:
 	const static int BACKLOG = 50;
 	const static int MAX_CLIENTS = 1;

 	struct pollfd poll_list[MAX_CLIENTS];
 	int proxy_socket;
 	int poll_size;

	int init_socket(int port);
 	void add_proxy();
 	void add_client();
 	// void add_to_poll(int socket, Client * client);
 	// void remove_dead();
};

#endif