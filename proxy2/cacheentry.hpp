#ifndef ENTRY_H
#define ENTRY_H

#include <cstddef>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <utility>
#include <ctime>

class CacheEntry {
public:
	CacheEntry(std::string url);
	~CacheEntry();

	char * get_data(size_t index);
	void append_data( char * buffer, int length);
	size_t get_length(size_t index) const;
	size_t get_full_length() const;
	size_t get_total() const;
	std::string get_url() const;

	bool is_finished() const;
	void set_finished();

	void add_reader();
	void remove_reader();
	bool is_used() const;

	void update_timestamp(std::time_t new_timestamp);
	std::time_t get_timestamp() const;

	pthread_mutex_t * get_entry_mutex();
	pthread_cond_t * get_entry_cond();
private:
	std::vector< std::pair<char *, int> > chunks;

	bool finished;
	int readers;
	std::string url;
	std::time_t timestamp;

	pthread_mutex_t entry_mutex;
	pthread_cond_t entry_cond;
};

#endif