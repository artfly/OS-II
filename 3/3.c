#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

char strbuf[4][256] = { "Test string #1" , "Test string #2" , "Test string #3" , "Test string #4" };
    
void *thread_body(void * param)
{
    printf( "%s\n" , (char*)param );
    pthread_exit(NULL);   
}

int main(int argc, char *argv[])
{
    pthread_t child_thread;
    int code;
    int i;
    
    for( i=0 ; i<4 ; i++ )
    {
    code=pthread_create(&child_thread, NULL, thread_body, (void*)strbuf[i] );
    if (code!=0)
    {
            printf( "Error creating thread\n");
        exit(1);
    }
    }
    pthread_exit(NULL);
    return (EXIT_SUCCESS);
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

char strbuf[4][256] = { "string 1" , "string 2" , "string 3" , "string 4" };
    
void *thread_body(void * param) {
    printf( "%s\n", (char*)param);
    pthread_exit(NULL);   
}

int main(int argc, char *argv[]) {
    pthread_t child_thread;
    int code;
    int i;
    
    for(i = 0; i < 4; i++) {
    code=pthread_create(&child_thread, NULL, thread_body, (void*)strbuf[i]);
        if (code!=0) {
            printf( "Error creating thread\n");
            exit(1);
        }
    }
    pthread_exit(NULL);
    return (EXIT_SUCCESS);
}
