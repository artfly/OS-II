#include "cacheentry.hpp"
#include "cache.hpp"

CacheEntry::CacheEntry() : entry_count(0), finished(false), readers(1) {
	pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&readers_mutex, &attr);
    pthread_mutex_init(&entry_mutex, &attr);
    pthread_cond_init(&entry_cond, NULL);
}

CacheEntry::~CacheEntry() {
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
	Cache::get_instance()->increase_size(len);
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

void CacheEntry::add_reader() {
	readers++;
	// std::cout << "DEBUG : Wow, how did this happen? Readers now : " << readers << std::endl;
}

void CacheEntry::remove_reader() {
	readers--;
}

bool CacheEntry::is_used() const {
	std::cout << "DEBUG : readers on is_used " << readers << std::endl;
	return readers != 0;
}

pthread_mutex_t * CacheEntry::get_readers_mutex() {
	return & readers_mutex;
}

pthread_cond_t * CacheEntry::get_entry_cond() {
	return & entry_cond;
}

pthread_mutex_t * CacheEntry::get_entry_mutex() {
	return & entry_mutex;
}