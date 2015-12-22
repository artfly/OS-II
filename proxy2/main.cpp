#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "proxy.hpp"

static Proxy *proxy = NULL;

void signal_handler(int sig) {
	delete proxy;
	proxy = NULL;
	exit(0);
}

int main(int argc, char ** argv) {
	signal(SIGPIPE, SIG_IGN);
	sigset(SIGINT, signal_handler);
	if (argc != 2) {
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}
	int port = atoi(argv[1]);
	proxy = new Proxy(port);
	proxy->run();
	delete proxy;
	exit(0);
}