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
 	const static int MAX_THREADS_NUM = 256;

 	struct pollfd poll_list[MAX_CLIENTS];
 	int proxy_socket;
 	int poll_size;
 	static int threads_num;
 	static pthread_mutex_t proxy_mutex;
 	static pthread_cond_t proxy_cond;

	int init_socket(int port);
 	void add_proxy();
 	void add_client();
};

#endif