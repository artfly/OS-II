#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define CHECK_AFTER 1000

pthread_barrier_t barrier;
pthread_mutex_t mutex;

int THREADS_NUM;
double * sums;
int GOT_SIGINT = 0;
int LAST_ITER = 0;

void signal_handler() {
	GOT_SIGINT = 1;
	printf("SIGINT!\n");
}

void * thread_body(void * param) {
	int thread_num = *((int *) param) + 1;
	int iteration = 0;
	double partial_sum = 0;
	int sign = 1;
	double element = 1.0 / (2 * thread_num - 1);
	if (thread_num % 2 == 0) {
		sign = -1;
		element *= -1;
	}
	while(1) {
		partial_sum += element;
		iteration++;
		if (iteration % CHECK_AFTER == 0) {
			pthread_barrier_wait(&barrier);
			pthread_mutex_lock(&mutex);
			if (GOT_SIGINT && iteration == LAST_ITER) {
				printf("%d\n", thread_num);
				sums[thread_num - 1] = partial_sum;
				pthread_mutex_unlock(&mutex);
				break;
			}
			if (GOT_SIGINT) {
				LAST_ITER = iteration + CHECK_AFTER;
			}
			pthread_mutex_unlock(&mutex);
		}
		int element_num = iteration * THREADS_NUM + thread_num;
		if (element_num % 2 == 0) {
			sign = -1;
		}
		else {
			sign = 1;
		}
		element = sign * 1.0 / (2 * element_num - 1);
	}
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
	struct sigaction act = {.sa_handler = signal_handler};
	sigaction(SIGINT, &act, NULL);
 	int i = 0;
 	int code;
 	double pi = 0;
 	if (argc != 2) {
 		printf("Usage : %s <threads_num>\n", argv[0]);
 		exit(EXIT_FAILURE);
 	}
 	THREADS_NUM = atoi(argv[1]);
 	if (THREADS_NUM < 1) {
 		printf("error : must be one or more threads\n");
 		exit(EXIT_FAILURE);
 	}
 	pthread_t * threads = (pthread_t *) malloc(THREADS_NUM * sizeof(pthread_t));
 	pthread_barrier_init(&barrier, NULL, THREADS_NUM);
 	pthread_mutex_init(&mutex, NULL);
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
 	pthread_barrier_destroy(&barrier);
 	printf("Pi = %.10f\n", pi * 4);
 	clear(threads, index, sums);
 	exit(EXIT_SUCCESS);
}