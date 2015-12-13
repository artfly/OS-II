#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex {
 public:
 	static Mutex * get_instance();
 	pthread_mutex_t * get_socket_mutex();
 	pthread_mutex_t * get_cache_mutex();
 private:
 	static Mutex * instance;
 	pthread_mutex_t socket_mutex;
 	pthread_mutex_t cache_mutex;
	Mutex();
	~Mutex();
};

#endif