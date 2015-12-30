#include <iostream>
#include <map>
#include <ctime>

int main(int argc, char ** argv) {
	std::multimap<std::time_t, int> timestamps;
	std::time_t timestamp = std::time(NULL);
	timestamps.insert(std::pair<time_t, int>(timestamp + 10, 42));
	timestamps.insert(std::pair<time_t, int>(timestamp, 24));
	for (std::multimap<std::time_t, int>::iterator it = timestamps.begin(); it != timestamps.end(); ++it) {
		std::cout << it->first << std::endl;
	}
	return 0;
}