#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

class Mutex {
 public:
 	static Mutex * get_instance();

 	pthread_mutex_t * get_finished_mutex();
 private:
 	static Mutex * instance;
 	pthread_mutex_t finished_mutex;
	Mutex();
	~Mutex();
};

#endif