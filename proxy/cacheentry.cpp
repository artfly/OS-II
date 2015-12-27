#include "cacheentry.hpp"
#include "cache.hpp"

CacheEntry::CacheEntry(std::string url) : finished(false),  readers(1), url(url) {
	timestamp = std::time(NULL);
}

CacheEntry::~CacheEntry() {
	std::cout << "DTOR" << std::endl;
	for (size_t i = 0; i < chunks.size(); i++) {
		free(chunks[i].first);
	}
}

char * CacheEntry::get_data(size_t index) {
	if (index >= chunks.size())
		return NULL;
	return chunks[index].first;
}

size_t CacheEntry::get_length(size_t index) const {
	if (index >= chunks.size())
		return 0;
	return chunks[index].second;
}

size_t CacheEntry::get_full_length() const {
	size_t full = 0;
	for (size_t i = 0; i < chunks.size(); i++) {
		full += chunks[i].second;
	}
	return full;
}

void CacheEntry::append_data(char * buffer, int len) {
	char * copy = (char *) malloc(len);
	memcpy(copy, buffer, len);
	chunks.push_back(std::make_pair(copy, len));
	// Cache::get_instance()->increase_size(len);
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

void CacheEntry::add_reader() {
	std::cout << "DEBUG : add_reader" << std::endl;
	readers++;
}

void CacheEntry::remove_reader() {
	std::cout << "DEBUG : remove_reader" << std::endl;
	readers--;
}

bool CacheEntry::is_used() const {
	return readers != 0;
}

std::time_t CacheEntry::get_timestamp() const {
	return timestamp;
}

void CacheEntry::update_timestamp(std::time_t new_timestamp) {
	timestamp = new_timestamp;
}

std::string CacheEntry::get_url() const {
	return url;
}