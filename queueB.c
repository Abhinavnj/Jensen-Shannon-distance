#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "queueB.h"

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

int initB(queueB_t *Q)
{
	Q->count = 0;
	Q->head = 0;
	Q->open = 1;
	pthread_mutex_init(&Q->lock, NULL);
	pthread_cond_init(&Q->read_ready, NULL);
	pthread_cond_init(&Q->write_ready, NULL);
	
	return 0;
}

int destroyB(queueB_t *Q)
{
	pthread_mutex_destroy(&Q->lock);
	pthread_cond_destroy(&Q->read_ready);
	pthread_cond_destroy(&Q->write_ready);

	return 0;
}


// add item to end of queue
// if the queue is full, block until space becomes available
int enqueueB(queueB_t *Q, char* item)
{
	lock(&Q->lock);
	
	while (Q->count == QSIZE && Q->open) {
		pthread_cond_wait(&Q->write_ready, &Q->lock);
	}

	if (!Q->open) {
		unlock(&Q->lock);
		return -1;
	}
	
	unsigned i = Q->head + Q->count;
	if (i >= QSIZE) i -= QSIZE;
	
    ++Q->count;

    int len = strlen(item) + 1;
    Q->data[i] = malloc(len);
    strcpy(Q->data[i], item);
    // printf("added: %s\n", Q->data[i]);
    // printB(Q);
	
	pthread_cond_signal(&Q->read_ready);
	
	unlock(&Q->lock);
	
	return 0;
}


int dequeueB(queueB_t *Q, char** item)
{
	lock(&Q->lock);
	
	while (Q->count == 0 && Q->open) {
		pthread_cond_wait(&Q->read_ready, &Q->lock);
	}

	if (Q->count == 0) {
		unlock(&Q->lock);
		return -1;
	}
	
    *item = malloc(strlen(Q->data[Q->head]) + 1);
    strcpy(*item, Q->data[Q->head]);
    free(Q->data[Q->head]);

    --Q->count;
    ++Q->head;
    if (Q->head == QSIZE) Q->head = 0;
	
	pthread_cond_signal(&Q->write_ready);
	
	unlock(&Q->lock);
	
	return 0;
}

int qcloseB(queueB_t *Q)
{
	lock(&Q->lock);
	Q->open = 0;
	pthread_cond_broadcast(&Q->read_ready);
	pthread_cond_broadcast(&Q->write_ready);
	unlock(&Q->lock);	

	return 0;
}

void printB(queueB_t* Q) {
    printf("File Queue: ");
    for (int i = 0; i < Q->count; i++) {
        printf("%s\t", Q->data[i]);
    }
    printf("\n");
}