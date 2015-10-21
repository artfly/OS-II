#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

sem_t sem1;
sem_t sem2;

void * thread_body(void * param) {
    int i;

    for (i = 0; i < 10; ++i) {
        sem_wait(&sem2);
        printf("Child %d\n", i);
        sem_post(&sem1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    int i;
    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 0);
    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    for (i = 0; i < 10; ++i) {
        sem_wait(&sem1);
        printf("Parent %d\n", i);
        sem_post(&sem2);
    }
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    return EXIT_SUCCESS;
}