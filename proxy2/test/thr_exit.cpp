#include "pthread.h"
#include "stdlib.h"
#include "unistd.h"
#include <iostream>


class A {
 public:
	A(int data) {
		important_int = data;
	}
	int get_int() const {
		return important_int;
	}
 private:
	int important_int;
};

class B {
 public:
 	static B * get_instance() {
 		if (!instance) {
 			instance = new B();
 		}
 		return instance;
 	}
 	void put_a (A * a) {
 		saved_a = a;
 	}
 	A * get_a (int num) {
 		if (saved_a && saved_a->get_int() == num) {
 			std::cout << "A returned!" << std::endl;
 			return saved_a;
 		}
 		std::cout << "Oops! No A." << std::endl;
 		return NULL;
 	}

 private:
 	B () {}
 	~B () {}
	static B * instance;
	A * saved_a;
};

B * B::instance = NULL;

void * thread_body(void * params) {
	A * a = new A(42);
	B * statc_b = B::get_instance();
	statc_b->put_a(a);
	return NULL;
}

int main(int argc, char ** argv) {
	pthread_t thread;
    int code;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    code = pthread_create(&thread, &attr, &thread_body, NULL);
    if (code != 0) {
    	std::cout << "error : couldn't create thread" << std::endl;
    	exit(EXIT_FAILURE);
    }

    B * b = B::get_instance();

    sleep(5);
    for (int i = 0; i < 10; i++) {
    	A * random_a = new A(24);
	}

   b->get_a(42);
   exit(EXIT_SUCCESS);
}