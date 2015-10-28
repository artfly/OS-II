#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond;

void * thread_body(void * param) {
    int i;
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    for (i = 0; i < 10; ++i) {
        pthread_cond_wait(&cond, &mutex);
        printf("Child : %d\n", i);
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    pthread_mutexattr_t attr;
    pthread_cond_init(&cond, NULL);
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex, &attr);
    int code;
    int i;
    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    //wait for thread to start
    pthread_mutex_lock(&mutex);
    for (i = 0; i < 10; ++i) {
        pthread_cond_wait(&cond, &mutex);
        printf("Parent %d\n", i);
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}