#include "cache.hpp"

Cache *Cache::instance = NULL;

Cache::Cache() : cur_size(0) {}

Cache::~Cache() {
	std::map<std::string, CacheEntry *>::iterator it = entries.begin();
  	for (it = entries.begin(); it != entries.end(); ++it) {
  		// std::cout << "DEBUG : deleting url : " << it->first << std::endl;
  		delete it->second;
  	}
}

void Cache::reset_instance() {
	delete instance;
	instance = NULL;
}

Cache * Cache::get_instance() {
	if (!instance) {
		instance = new Cache();
	}
	return instance;
}

bool Cache::put_entry(CacheEntry * entry, int size) {
	std::cout << "DEBUG : size " << size << " cur_size : " << cur_size << std::endl;
	if (size < 0)
		return false;
	if (MAX_CACHE_SIZE - cur_size < size) {
		remove_oldest(size);
	}
	if (MAX_CACHE_SIZE - cur_size < size)
		return false;
	std::cout << "DEBUG : adding entry to cache" << std::endl;
	cur_size += size;
	entries[entry->get_url()] = entry;
	timestamps.insert(std::pair<std::time_t, CacheEntry *>(entry->get_timestamp(), entry));
	return true;
}

void Cache::update_timestamp(CacheEntry * entry) {
	if (!entry)
		return;
	std::time_t old_timestamp = entry->get_timestamp();
	std::time_t new_timestamp = std::time(NULL);
	std::pair<std::multimap<time_t, CacheEntry *>::iterator, std::multimap<time_t, CacheEntry *>::iterator> range;
	range = timestamps.equal_range(old_timestamp);
	for (std::multimap<time_t, CacheEntry *>::iterator it = range.first; it != range.second; ++it) {
		if (it->second == entry) {
			timestamps.erase(it);
			timestamps.insert(std::pair<std::time_t, CacheEntry *>(new_timestamp, entry));
			return;
		}
	}
	entry->update_timestamp(new_timestamp);
}

void Cache::remove_oldest(int added) {
	std::cout << "DEBUG : here " << std::endl;
	int need_to_remove = REMOVE_SIZE;
	int removed = 0;
	if (added > need_to_remove) need_to_remove = added;
	std::multimap<std::time_t, CacheEntry *>::iterator it;
	for (it = timestamps.begin(); it != timestamps.end(); ++it) {
		if (removed >= need_to_remove) {
			cur_size -= removed;
			return;
		}
		if (!it->second->is_used()) {
			std::cout << "DEBUG : unused entry : " << it->second->get_url() << std::endl;
			removed += it->second->get_full_length();
			delete it->second;
			entries.erase(it->second->get_url());
			timestamps.erase(it);
		}
	}
	cur_size -= removed;
}

CacheEntry * Cache::get_entry(std::string url) {
	std::map<std::string, CacheEntry *>::iterator it = entries.find(url);
	if (it == entries.end()) {
		return NULL;
	}
	std::cout << "Getting entry from cache : " << url << std::endl;
	return it->second;
}