#include "cache.hpp"

Cache *Cache::instance = NULL;

Cache::Cache() {}

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
	if (entries.size() + 1 > MAX_CACHE_SIZE) {
		remove_oldest();
	}
	entries[url] = entry;
	entry->set_entry_count(entries.size());
}

void Cache::remove_oldest() {
	std::vector< std::pair<std::string, CacheEntry *> > oldest;
	std::map<std::string, CacheEntry *>::iterator it = entries.begin();
	while (it != entries.end()) {

		if (oldest.size() < 10) {
			oldest.push_back(std::make_pair(it->first, it->second));
			continue;
		}

		for(std::vector< std::pair<std::string, CacheEntry *> >::iterator oldest_it =
									oldest.begin(); oldest_it != oldest.end(); ++oldest_it) {
			if (oldest_it->second->get_entry_count() > it->second->get_entry_count()) {
				*oldest_it = std::make_pair(it->first, it->second);
			}
		}
  	}

  	for(std::vector< std::pair<std::string, CacheEntry *> >::iterator it =
  												oldest.begin(); it != oldest.end(); ++it) {
  		entries.erase(it->first);
	}
}

CacheEntry * Cache::get_entry(const std::string & url) const {
	std::map<std::string, CacheEntry *>::const_iterator it = entries.find(url);
	if (it == entries.end()) {
		return NULL;
	}
	std::cout << "Getting entry from cache" << std::endl;
	return it->second;
}

// int main(int argc, char ** argv) {
// 	Cache * cache = Cache::get_instance();
// 	CacheEntry * entry = new CacheEntry();
// 	std::string test = "TEST_DATA";
// 	entry->append_data(test.c_str(), test.length());
// 	std::string url = "www.fakeurl.com/test";
// 	cache->put_entry(url, entry);
// 	for (auto i = 0; i < 10; i++) {
// 		cache->put_entry(url, entry);
// 		if (i == 9) {
// 			cache->print_entries();
// 			std::cout << "AND NOW REMOVE" << std::endl;
// 		}
// 		cache->print_entries();
// 	}
// 	return 0;
// }