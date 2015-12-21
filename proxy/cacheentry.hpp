#ifndef ENTRY_H
#define ENTRY_H

#include <cstddef>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility>

class Cache;

class CacheEntry {
public:
	CacheEntry();
	~CacheEntry();

	char * get_data(size_t index);
	void append_data( char * buffer, int length);
	size_t get_length(size_t index) const;
	size_t get_full_length() const;
	size_t get_total() const;

	size_t get_entry_count() const;
	void set_entry_count(size_t count);

	bool is_finished() const;
	void set_finished();
	void set_cache(Cache * cache);
	void add_reader();
	void remove_reader();
	bool is_used() const;
private:
	std::vector< std::pair<char *, int> > chunks;

	Cache * cache;
	size_t entry_count;
	bool finished;
	int readers;
};

#endif