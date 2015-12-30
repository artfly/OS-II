#ifndef CLIENT_H
#define CLIENT_H

#include <poll.h>
#include "connection.hpp"
#include "requestheader.hpp"
#include "responseheader.hpp"
#include "cacheentry.hpp"
#include "mutex.hpp"
#include "cache.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <netdb.h>
#include <errno.h>


enum ClientState {
	CONNECT_REMOTE, READ_CACHE, SEND_ERROR, EXIT_CLIENT
};

enum RemoteState {
	 WAIT, REQUEST_HEADER, READ_HEADER, READ_CONTENT, EXIT_REMOTE
};

class Client {
 public:
	Client(int sock);
	~Client();
	void work(short events, int socket);
	bool alive() const;
	int get_sock(int sock) const;
	void run();
 private:
 	const static int MAX_CLIENTS = 2;
 	struct pollfd poll_list[MAX_CLIENTS];
 	int poll_size;
 	void create_poll();
 	void add_to_poll(int sock, int mode);
 	void remove_from_poll(int sock);

 	ClientState state;
 	RemoteState remote_state;
 	int chunk_to_read;
 	int total;
 	int requested_header;
 	bool in_cache;

 	CacheEntry * entry;
 	Cache * cache;

	void client_work(short events);
	void connect_server();

	void remote_work(short events);
	bool read_remote_header();
	bool read_remote_data();

 	Connection * client_conn;
 	Connection * remote_conn;
 	RequestHeader * request_header;
 	ResponseHeader * response_header;

};


#endif