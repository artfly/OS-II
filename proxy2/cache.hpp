#ifndef CACHE_H
#define CACHE_H

#include <map>
#include <iostream>
#include <string>
#include "cacheentry.hpp"

class Cache {
public:
	static Cache * get_instance();
	void put_entry(const std::string & url, CacheEntry * entry);
	CacheEntry * get_entry(const std::string & url) const;
private:
	static const int MAX_CACHE_SIZE = 5;
	std::map < std::string, CacheEntry *> entries;

	Cache();
	~Cache();

	static Cache * instance;
	void remove_oldest();
};

#endif