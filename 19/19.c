#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#define STRLEN 80

struct node {
	struct node * next;
	char * str;
};
typedef struct node node_t;

pthread_rwlock_t rwlock;
node_t * head = NULL;

node_t * add_node(node_t * node, node_t * head) {
	node->next = head;
	return node;
} 

void show_list(node_t * head) {
	node_t * cur = head;
	while(cur) {
		printf("%s\n", cur->str);
		cur = cur->next;	
	}
}


void * sort_list(void * param) {
	while(1) {
		char * tmp;
		sleep(5);
		pthread_rwlock_wrlock(&rwlock);
		node_t * node1 = head;
		node_t * node2 = head;
		if (!head) {
			printf("my head is still empty\n");
		}
		while(node1) {
			while(node2->next) {
				if (strcmp(node2->next->str, node2->str) > 0) {
					tmp = node2->str;
					node2->str = node2->next->str;
					node2->next->str = tmp;
				}
				node2 = node2->next;
			}
			node1 = node1->next;
		}
		pthread_rwlock_unlock(&rwlock);
	}
}

int main(int argc, char * argv[]) {
	char buff[STRLEN];
	pthread_t thread;
	int code;
	pthread_rwlockattr_t attr;
	pthread_rwlockattr_init(&attr);
    pthread_rwlock_init(&rwlock, &attr);
    code = pthread_create(&thread, NULL, sort_list, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

	while(1) {
		if (!fgets(buff, STRLEN, stdin)) {
			exit(EXIT_FAILURE);
		}
		size_t l = strlen(buff);
		if (l != 1 || buff[l - 1] != '\n') {
			node_t * node = (node_t *) malloc(sizeof(node_t));
			node->str = malloc(sizeof(char) * STRLEN);
			strncpy(node->str, buff, sizeof(char) * (l-1));
			pthread_rwlock_wrlock(&rwlock);
			head = add_node(node, head);
			pthread_rwlock_unlock(&rwlock);
		}
		else {
			pthread_rwlock_rdlock(&rwlock);
			show_list(head);
			pthread_rwlock_unlock(&rwlock);
		}
	}

	exit(EXIT_SUCCESS);
}