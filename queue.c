#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "queue.h"

int init(queue_t *Q)
{
    Q->data = malloc(0);
	Q->count = 0;
	Q->head = 0;
	pthread_mutex_init(&Q->lock, NULL);
	
	return 0;
}

int destroy(queue_t *Q)
{
	pthread_mutex_destroy(&Q->lock);

    // for (int i = 0; i < Q->count; i++) {
    //     printf("hi");
    //     free(Q->data[i]);
    // }
    free(Q->data);

	return 0;
}


// add item to end of queue
// if the queue is full, block until space becomes available
int enqueue(queue_t *Q, char* item)
{
	pthread_mutex_lock(&Q->lock);

    // realloc
    ++Q->count;
    Q->data = realloc(Q->data, sizeof(Q->data[0]) * Q->count); // reallocating number of rows
	
    int i = Q->count - 1;
    Q->data[i] = malloc(sizeof(item));
    Q->data[i] = item;
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}


int dequeue(queue_t *Q, char** item)
{
	pthread_mutex_lock(&Q->lock);
	
	*item = Q->data[Q->count - 1];
	--Q->count;
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}