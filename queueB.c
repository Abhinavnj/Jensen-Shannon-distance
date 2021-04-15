#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queueB.h"

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
	pthread_mutex_lock(&Q->lock);
	
	while (Q->count == QSIZE && Q->open) {
		pthread_cond_wait(&Q->write_ready, &Q->lock);
	}
	if (!Q->open) {
		pthread_mutex_unlock(&Q->lock);
		return -1;
	}
	
	unsigned i = Q->head + Q->count;
	if (i >= QSIZE) i -= QSIZE;
	
	Q->data[i] = item;
	++Q->count;
	
	pthread_cond_signal(&Q->read_ready);
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}


int dequeueB(queueB_t *Q, char** item)
{
	pthread_mutex_lock(&Q->lock);
	
	while (Q->count == 0 && Q->open) {
		pthread_cond_wait(&Q->read_ready, &Q->lock);
	}
	if (Q->count == 0) {
		pthread_mutex_unlock(&Q->lock);
		return -1;
	}
	
	*item = Q->data[Q->head];
	--Q->count;
	++Q->head;
	if (Q->head == QSIZE) Q->head = 0;
	
	pthread_cond_signal(&Q->write_ready);
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}

int qcloseB(queueB_t *Q)
{
	pthread_mutex_lock(&Q->lock);
	Q->open = 0;
	pthread_cond_broadcast(&Q->read_ready);
	pthread_cond_broadcast(&Q->write_ready);
	pthread_mutex_unlock(&Q->lock);	

	return 0;
}

void printB(queueB_t* Q) {
    for (int i = 0; i < Q->count; i++) {
        printf("%s\t", Q->data[i]);
    }
    printf("\n");
}