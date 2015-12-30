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
	bool put_entry(CacheEntry * entry, int size);
	CacheEntry * get_entry(std::string url);
	pthread_mutex_t * get_cache_mutex();
	void update_timestamp(CacheEntry * entry);
private:
	static const int MAX_CACHE_SIZE = 50000000;					//50 mb
	static const int REMOVE_SIZE = 5000000;						//5 mb
	static Cache * instance;

	Cache();
	~Cache();

	std::map < std::string, CacheEntry *> entries;
	std::multimap<std::time_t, CacheEntry *> timestamps;
	int cur_size;
	pthread_mutex_t cache_mutex;

	void remove_oldest(int added);
};

#endif