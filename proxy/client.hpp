#ifndef CLIENT_H
#define CLIENT_H

#include <poll.h>
#include "connection.hpp"
#include "requestheader.hpp"
#include "responseheader.hpp"
#include "cacheentry.hpp"
#include "cache.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <netdb.h>
#include <errno.h>


enum ClientState {
	CONNECT_REMOTE, READ_CACHE, EXIT_CLIENT
};

enum RemoteState {
	 WAIT, READ_HEADER, READ_CONTENT, EXIT_REMOTE
};

class Client {
 public:
	Client(int sock);
	~Client();
	int work(short events, int socket);
	bool alive() const;
	int get_sock(int sock) const;
 private:
 	ClientState state;
 	RemoteState remote_state;
 	int chunk_to_read;
 	int total;
 	int requested_header;

 	CacheEntry * entry;
 	Cache * cache;

	int client_work(short events);
	void connect_server();

	int remote_work(short events);
	bool read_remote_header();
	bool read_remote_data();

 	Connection * client_conn;
 	Connection * remote_conn;
 	RequestHeader * request_header;
 	ResponseHeader * response_header;

};


#endif