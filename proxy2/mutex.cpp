#include "mutex.hpp"

Mutex * Mutex::instance = NULL;

Mutex::Mutex() {
	pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&finished_mutex, &attr);
}

Mutex::~Mutex() {
	pthread_mutex_destroy(&finished_mutex);
}

Mutex * Mutex::get_instance() {
	if (!instance)
		instance = new Mutex();
	return instance;
} 


pthread_mutex_t * Mutex::get_finished_mutex() {
	return &finished_mutex;
}