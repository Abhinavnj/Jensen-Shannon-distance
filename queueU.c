#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "queueU.h"

#define lock(X) \
        do { \
            int err = pthread_mutex_lock(X); \
            if (err) { \
                perror("lock"); \
                abort(); \
            } \
        } while (0) \

#define unlock(X) \
        do { \
            int err = pthread_mutex_unlock(X); \
            if (err) { \
                perror("unlock"); \
                abort(); \
            } \
        } while (0) \

int initU(queueU_t *Q)
{
    Q->data = malloc(0);
	Q->count = 0;
	Q->head = 0;
	pthread_mutex_init(&Q->lock, NULL);
    pthread_cond_init(&Q->read_ready, NULL);
	
	return 0;
}

int destroyU(queueU_t *Q)
{
	pthread_mutex_destroy(&Q->lock);

    free(Q->data);

	return 0;
}


// add item to end of queue
int enqueueU(queueU_t *Q, char* item)
{
	lock(&Q->lock);

    // realloc
    ++Q->count;
    Q->data = realloc(Q->data, sizeof(Q->data[0]) * Q->count); // reallocating number of rows
	
    int i = Q->count - 1;
    int len = strlen(item) + 1;
    Q->data[i] = malloc(len);
    strcpy(Q->data[i], item);

    // free(item);

    pthread_cond_signal(&Q->read_ready);
	
	unlock(&Q->lock);
	
	return 0;
}


int dequeueU(queueU_t *Q, char** item)
{
	lock(&Q->lock);

    while (Q->count == 0) {
		pthread_cond_wait(&Q->read_ready, &Q->lock);
	}
	
	*item = Q->data[Q->count - 1];
    // free(Q->data[Q->count - 1]);
	--(Q->count);
	
	unlock(&Q->lock);
	
	return 0;
}

void printU(queueU_t* Q) {
    for (int i = 0; i < Q->count; i++) {
        printf("%s\t", Q->data[i]);
    }
    printf("\n");
}