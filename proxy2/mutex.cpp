#include "mutex.hpp"

Mutex * Mutex::instance = NULL;

Mutex::Mutex() {
	pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&socket_mutex, &attr);
    pthread_mutex_init(&cache_mutex, &attr);
}

Mutex::~Mutex() {
	pthread_mutex_destroy(&socket_mutex);
	pthread_mutex_destroy(&cache_mutex);
}

Mutex * Mutex::get_instance() {
	if (!instance)
		instance = new Mutex();
	return instance;
} 

pthread_mutex_t * Mutex::get_socket_mutex() {
	return &socket_mutex;
}

pthread_mutex_t * Mutex::get_cache_mutex() {
	return &cache_mutex;
}