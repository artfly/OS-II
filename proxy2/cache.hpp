#ifndef CACHE_H
#define CACHE_H

#include <map>
#include <iostream>
#include <string>
#include "cacheentry.hpp"

class Cache {
public:
	static Cache * get_instance();
	static void reset_instance();
	void put_entry(const std::string & url, CacheEntry * entry);
	CacheEntry * get_entry(const std::string & url) const;
	void increase_size(int size);
	pthread_mutex_t * get_cache_mutex();
private:
	static const int MAX_CACHE_SIZE = 2000000;					//50 mb
	static const int REMOVE_SIZE = 200000;						//5 mb
	std::map < std::string, CacheEntry *> entries;
	int cur_size;
	pthread_mutex_t cache_mutex;

	Cache();
	~Cache();

	static Cache * instance;
	void remove_oldest();
};

#endif