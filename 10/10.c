#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

void * thread_body(void * param) {
    int i;
    pthread_mutex_lock(&mutex2);

    for (i = 0; i < 10; ++i) {
        pthread_mutex_lock(&mutex3);
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_lock(&mutex1);
        printf("Child : %d\n", i);
        pthread_mutex_unlock(&mutex3);
        pthread_mutex_lock(&mutex2);
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex1, &attr);
    pthread_mutex_init(&mutex2, &attr);
    pthread_mutex_init(&mutex3, &attr);
    int code;
    int i;
    pthread_mutex_lock(&mutex3);
    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    //wait for thread to start
    sleep(5);
    
    for (i = 0; i < 10; ++i) {
        pthread_mutex_lock(&mutex1);
        printf("Parent %d\n", i);
        pthread_mutex_unlock(&mutex3);
        pthread_mutex_lock(&mutex2);
        pthread_mutex_unlock(&mutex1);
        pthread_mutex_lock(&mutex3);
        pthread_mutex_unlock(&mutex2);
    }
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);

    return EXIT_SUCCESS;
}