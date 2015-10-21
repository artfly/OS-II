#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define STEPS 200000000						//TODO check prog

int THREADS_NUM;
double * sums;

void * thread_body(void * param) {
	int * thread_num = (int *) param;
	int thread_steps = STEPS / THREADS_NUM;
	int offset = *thread_num * thread_steps;
	int add = 2;
	int denominator = (offset + 1) * 2 - 1;
	if (*thread_num % 2 != 0) {
		denominator *= -1;
		add *= -1;
	}
	int i = 0;
	double partial_sum = 0;
	for (i = 0; i < thread_steps; i++) {
		partial_sum += 1.0 / denominator;
		denominator = -1 * (denominator + add);
		add *= -1;
	}
	sums[*thread_num] = partial_sum;
	pthread_exit(NULL);
}	

void handle_error(pthread_t * threads, int created, int code) {
	int i = 0;
	for (i = 0; i < created; i++) {
		pthread_join(threads[i], NULL);
	}
	char buf[256];
	strerror_r(code, buf, sizeof buf);
	printf("error in %d thread creation\n", i);
}

void clear(pthread_t * threads, int * index, double * sums) {
	free(threads);
 	free(index);
 	free(sums);
}


int main(int argc, char *argv[]) {
 	int i = 0;
 	int code;
 	double pi = 0;
 	if (argc != 2) {
 		printf("Usage : %s <threads_num>\n", argv[0]);
 		exit(EXIT_FAILURE);
 	}
 	THREADS_NUM = atoi(argv[1]);
 	if (THREADS_NUM < 1 || THREADS_NUM > STEPS) {
 		printf("error : threads_num ranges from 1 to %d\n", STEPS);
 		exit(EXIT_FAILURE);
 	}
 	pthread_t * threads = (pthread_t *) malloc(THREADS_NUM * sizeof(pthread_t));
 	int * index = (int *) calloc(THREADS_NUM, sizeof(int));
 	for (i = 0; i < THREADS_NUM; i++) {
 		index[i] = i;
 	}
	sums = (double *) malloc(THREADS_NUM * sizeof(double));
 	for (i = 0; i < THREADS_NUM; i++) {
 		code = pthread_create(&threads[i], NULL, thread_body, (void *)&index[i]);
    	if (code != 0) {
    		handle_error(threads, i, code);
		  	clear(threads, index, sums);
		    exit(EXIT_FAILURE);																				
    	}
 	}
 	for (i = 0; i < THREADS_NUM; i++) {
 		pthread_join(threads[i], NULL);
 		pi += sums[i];
 	}
 	printf("Pi = %.10f\n", pi * 4);
 	clear(threads, index, sums);
 	exit(EXIT_SUCCESS);
}