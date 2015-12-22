#include "cache.hpp"

Cache *Cache::instance = NULL;

Cache::Cache() : cur_size(0) {
	pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&cache_mutex, &attr);
}

Cache::~Cache() {
	std::cout << "cache Dtor" << std::endl;
	std::map<std::string, CacheEntry *>::iterator it = entries.begin();
  	while (it != entries.end()) {
  		delete it->second;
  	}
  	std::cout << "cache dtor end" << std::endl;
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

void Cache::put_entry(const std::string & url, CacheEntry * entry) {
	entries[url] = entry;
	entry->set_entry_count(entries.size());
	// entry->set_cache(this);
}

void Cache::remove_oldest() {
	std::cout << "DEBUG : size before remove : " << cur_size << std::endl;
	std::vector< std::pair<std::string, CacheEntry *> > oldest;
	std::map<std::string, CacheEntry *>::iterator it;
	int removed = 0;
	for (it = entries.begin(); it != entries.end(); ++it) {
		if (removed >= REMOVE_SIZE)
			return;
		pthread_mutex_t * rmutex = it->second->get_readers_mutex();
		pthread_mutex_lock(rmutex);
		std::cout << "DEBUG : zero readers? " << !it->second->is_used() << std::endl;
		std::cout << "DEBUG : url : " << it->first << std::endl;
		if (!it->second->is_used()) {
			//mutex?
			removed += it->second->get_full_length();
			delete it->second;
			entries.erase(it->first);
		}
		pthread_mutex_unlock(rmutex);
	}
	cur_size -= removed;
	std::cout << "DEBUG : size after remove : " << cur_size << std::endl;
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
	return it->second;
}

void Cache::increase_size(int size) {
	pthread_mutex_lock(&cache_mutex);
	cur_size += size;
	// std::cout << "DEBUG : adding cur_size" << std::endl;

	if (cur_size > MAX_CACHE_SIZE) {
		// std::cout << "DEBUG : removing" << std::endl;
		remove_oldest();
	}
	std::cout << "DEBUG : new cache size : " << cur_size << std::endl;
	pthread_mutex_unlock(&cache_mutex);
	// std::cout << "DEBUG : finished increasing" << std::endl;
}

pthread_mutex_t * Cache::get_cache_mutex() {
	return &cache_mutex;
}