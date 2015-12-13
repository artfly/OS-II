#ifndef CONNECTION_H
#define CONNECTION_H

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <unistd.h>


class Connection {
 public:
	Connection(int sock);
	~Connection();

	int recv_data();
	int send_data(const char * data, int len);

	char * get_buffer();
	int get_length() const;

	void close_sock();
	void set_sock(int s);
	int get_sock() const;
 private:
 	static const int BUFLEN = 65536;
 	int len;
 	char * buffer;
 	int sock;
 	// std::vector< std::pair<char *, int> > chunks;
};


#endif