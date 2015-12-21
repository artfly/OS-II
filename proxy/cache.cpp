#include "cache.hpp"

Cache *Cache::instance = NULL;

Cache::Cache() : cur_size(0) {}

Cache::~Cache() {
	std::map<std::string, CacheEntry *>::iterator it = entries.begin();
  	while (it != entries.end()) {
  		delete it->second;
  	}
}

Cache * Cache::get_instance() {
	if (!instance) {
		instance = new Cache();
	}
	return instance;
}

void Cache::put_entry(const std::string & url, CacheEntry * entry) {
	entries[url] = entry;
	entry->set_entry_count(entries.size());
	entry->set_cache(this);
}

void Cache::remove_oldest() {
	std::cout << "DEBUG : removing oldest entries" << std::endl;
	std::vector< std::pair<std::string, CacheEntry *> > oldest;
	std::map<std::string, CacheEntry *>::iterator it;
	int removed = 0;
	for (it = entries.begin(); it != entries.end(); ++it) {
		if (removed >= REMOVE_SIZE)
			return;
		if (!it->second->is_used()) {
			removed += it->second->get_full_length();
			delete it->second;
			entries.erase(it->first);
		}
	}
	// while (it != entries.end()) {

	// 	if (oldest.size() < 10) {
	// 		oldest.push_back(std::make_pair(it->first, it->second));
	// 		continue;
	// 	}

	// 	for(std::vector< std::pair<std::string, CacheEntry *> >::iterator oldest_it =
	// 								oldest.begin(); oldest_it != oldest.end(); ++oldest_it) {
	// 		if (oldest_it->second->get_entry_count() > it->second->get_entry_count()) {
	// 			*oldest_it = std::make_pair(it->first, it->second);
	// 		}
	// 	}
 //  	}

 //  	for(std::vector< std::pair<std::string, CacheEntry *> >::iterator it =
 //  												oldest.begin(); it != oldest.end(); ++it) {
 //  		entries.erase(it->first);
	// }
}

CacheEntry * Cache::get_entry(const std::string & url) const {
	std::map<std::string, CacheEntry *>::const_iterator it = entries.find(url);
	if (it == entries.end()) {
		return NULL;
	}
	std::cout << "Getting entry from cache : " << url << std::endl;
	it->second->add_reader();
	return it->second;
}

void Cache::increase_size(int size) {
	std::cout << "DEBUG : adding cur_size : " << cur_size << std::endl;
	// Cache::get_instance();
	cur_size += size;
	std::cout << "DEBUG : adding cur_size" << std::endl;

	if (cur_size > MAX_CACHE_SIZE) {
		std::cout << "DEBUG : removing" << std::endl;
		remove_oldest();
	}
	std::cout << "DEBUG : finished increasing" << std::endl;
}