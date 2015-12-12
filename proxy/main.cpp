#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "proxy.hpp"

int main(int argc, char ** argv) {
	signal(SIGPIPE, SIG_IGN);
	if (argc != 2) {
		printf("usage : %s <port>\n", argv[0]);
		exit(1);
	}
	int port = atoi(argv[1]);
	Proxy * proxy = new Proxy(port);
	proxy->run();
	delete proxy;
	exit(0);
}