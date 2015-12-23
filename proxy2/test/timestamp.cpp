#include <ctime>
#include <iostream>
#include <unistd.h>

int main() {
	 std::time_t result = std::time(NULL);
	 sleep(5);
	 std::time_t result2 = std::time(NULL);
	 std::cout << result - result2 << std::endl;
	 return 0;
}
