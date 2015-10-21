#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 64
#define STDOUT 1

void print_goodbye(void * arg) {
    printf("\nOh, no, Im being slain!\n");
}

void * thread_body(void * param) {
    pthread_cleanup_push(print_goodbye, NULL);
    char buf[BUF_SIZE] = "SPAM! ";
    while (1) {
        write(STDOUT, buf, BUF_SIZE);
    }
    pthread_cleanup_pop(0);
    pthread_exit(NULL);
}

void handle_error(int code) {
    char buf[256];
    strerror_r(code, buf, sizeof buf);
    fprintf(stderr, "error creating thread: %s\n", buf);
}


int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    void * res = NULL;

    code = pthread_create(& thread, NULL, thread_body, NULL);
    if (code != 0) {
        handle_error(code);
        exit(EXIT_FAILURE);
    }
    sleep(2);
    code = pthread_cancel(thread);
    if (code != 0) {
        handle_error(code);
        exit(EXIT_FAILURE);
    }
    code = pthread_join(thread, &res);
    if (code != 0) {
        handle_error(code);
        exit(EXIT_FAILURE);
    }
    if (res == PTHREAD_CANCELED) {
        printf("\nThread was canceled\n");
    }
    else {
        printf("\nerror : couldn't cancel the thread \n");
    }
    exit(EXIT_SUCCESS);
}