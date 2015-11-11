#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h> 

#define SNAME1 "/sem1"
#define SNAME2 "/sem2"

int main(int argc, char *argv[]) {
    sem_t * sem1 = sem_open(SNAME1, O_CREAT, 0644, 1);
    sem_t * sem2 = sem_open(SNAME2, O_CREAT, 0644, 0);
    int i;
    pid_t pid;
    if ((pid = fork()) == 0) {
        int i;
        sem_t * sem1 = sem_open(SNAME1, 0);
        sem_t * sem2 = sem_open(SNAME2, 0);
        for (i = 0; i < 10; ++i) {
            sem_wait(sem2);
            printf("Child %d\n", i);
            sem_post(sem1);
        }
        sem_close(&sem1);
        sem_close(&sem2);
    }
    else {
        for (i = 0; i < 10; ++i) {
            sem_wait(sem1);
            printf("Parent %d\n", i);
            sem_post(sem2);
        }
        sem_close(&sem1);
        sem_close(&sem2);
        sem_unlink(SNAME1);
        sem_unlink(SNAME2);
    }
    return EXIT_SUCCESS;
}