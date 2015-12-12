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

class CacheEntry {
public:
	CacheEntry();
	~CacheEntry();

	char * get_data(size_t index);
	void append_data( char * buffer, int length);
	size_t get_length(size_t index) const;
	size_t get_total() const;

	size_t get_entry_count() const;
	void set_entry_count(size_t count);

	bool is_finished() const;
	void set_finished();
private:
	std::vector< std::pair<char *, int> > chunks;
	size_t entry_count;
	bool finished;
};

#endif