#include "cacheentry.hpp"

CacheEntry::CacheEntry() : entry_count(0), finished(false) {}

CacheEntry::~CacheEntry() {
	for (size_t i = 0; i < chunks.size(); i++) {
		std::cout << "i" << std::endl;
		free(chunks[i].first);
	}
}

char * CacheEntry::get_data(size_t index) {
	if (index >= chunks.size())
		return NULL;
	std::cout << "GET DATA FROM ENTRY : " << chunks[index].first << std::endl;
	return chunks[index].first;
}

size_t CacheEntry::get_length(size_t index) const {
	if (index >= chunks.size())
		return 0;
	return chunks[index].second;
}

void CacheEntry::append_data(char * buffer, int len) {
	char * copy = (char *) malloc(len);
	memcpy(copy, buffer, len);
	chunks.push_back(std::make_pair(copy, len));
	std::cout << "DATA TO APPEND : " << std::endl;
	write(1, chunks.back().first, len);
	std::cout << '\n';
}

size_t CacheEntry::get_entry_count() const {
	return entry_count;
}

void CacheEntry::set_entry_count(size_t count) {
	entry_count = count;
}

bool CacheEntry::is_finished() const {
	return finished;
}

void CacheEntry::set_finished() {
	finished = true;
}

size_t CacheEntry::get_total() const {
	return chunks.size();
}