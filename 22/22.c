#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

sem_t sem_A, sem_B, sem_C, sem_module;

void * create_A(void * param) {
	while(1) {
		sleep(1);
		sem_post(&sem_A);
		printf("Created A!\n");
	}
}

void * create_B(void * param) {
	while(1) {
		sleep(2);
		sem_post(&sem_B);
		printf("Created B!\n");
	}
}

void * create_C(void * param) {
	while(1) {
		sleep(3);
		sem_post(&sem_C);
		printf("Created C!\n");
	}
}

void * create_module(void * param) {
	while(1) {
		sem_wait(&sem_A);
		sem_wait(&sem_B);
		sem_post(&sem_module);
		printf("Created module!\n");
	}
}

int main(int argc, char * argv[]) {
	pthread_t thread_A;
	pthread_t thread_B;
	pthread_t thread_C;
	pthread_t thread_module;

	sem_init(&sem_A, 0, 0);
	sem_init(&sem_B, 0, 0);
	sem_init(&sem_C, 0, 0);
	sem_init(&sem_module, 0, 0);

	pthread_create(&thread_A, NULL, create_A, NULL);
	pthread_create(&thread_B, NULL, create_B, NULL);
	pthread_create(&thread_C, NULL, create_C, NULL);
	pthread_create(&thread_module, NULL, create_module, NULL);

	while(1) {
		sem_wait(&sem_C);
		sem_wait(&sem_module);
		printf("Created new widget!\n");
	}
	exit(EXIT_SUCCESS);
}