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
private:
	static int MAX_CACHE_SIZE;					//50 mb
	static int REMOVE_SIZE;						//5 mb
	std::map < std::string, CacheEntry *> entries;
	int cur_size;

	Cache();
	~Cache();

	static Cache * instance;
	void remove_oldest();
};

#endif