#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#ifndef QSIZE
#define QSIZE 8
#endif

typedef struct {
	int data[QSIZE];
	unsigned count;
	unsigned head;
	pthread_mutex_t lock;
	pthread_cond_t read_ready;
	pthread_cond_t write_ready;
} queue_t;

int init(queue_t *Q)
{
	Q->count = 0;
	Q->head = 0;
	pthread_mutex_init(&Q->lock, NULL);
	pthread_cond_init(&Q->read_ready, NULL);
	pthread_cond_init(&Q->write_ready, NULL);
	
	return 0;
}

int destroy(queue_t *Q)
{
	pthread_mutex_destroy(&Q->lock);
	pthread_cond_destroy(&Q->read_ready);
	pthread_cond_destroy(&Q->write_ready);

	return 0;
}


// add item to end of queue
// if the queue is full, block until space becomes available
int enqueue(queue_t *Q, int item)
{
	pthread_mutex_lock(&Q->lock);
	
	while (Q->count == QSIZE) {
		pthread_cond_wait(&Q->write_ready, &Q->lock);
	}
	
	unsigned i = Q->head + Q->count;
	if (i >= QSIZE) i -= QSIZE;
	
	Q->data[i] = item;
	++Q->count;
	
	pthread_cond_signal(&Q->read_ready);
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}


int dequeue(queue_t *Q, int *item)
{
	pthread_mutex_lock(&Q->lock);
	
	while (Q->count == 0) {
		pthread_cond_wait(&Q->read_ready, &Q->lock);
	}
	
	*item = Q->data[Q->head];
	--Q->count;
	++Q->head;
	if (Q->head == QSIZE) Q->head = 0;
	
	pthread_cond_signal(&Q->write_ready);
	
	pthread_mutex_unlock(&Q->lock);
	
	return 0;
}


/* ******** 
 * Example client code for synchronized queue 
 */

struct targs {
	queue_t *Q;
	int id;
	int max;
	int wait;
};

void *producer(void *A) 
{
	struct targs *args = A;
	
	int i, start, stop;
	printf("[%d] begin producing %d\n", args->id, args->max);

	start = args->id * 1000;
	stop = start + args->max;
	for (i = start; i < stop; ++i) {
		printf("[%d] Sending %d\n", args->id, i);
		enqueue(args->Q, i);
		sleep(args->wait);
	}
	
	printf("[%d] finished\n", args->id);
	
	return NULL;
}


void *consumer(void *A)
{
	struct targs *args = A;
	int i, j;
	
	printf("[%d] begin consuming %d\n", args->id, args->max);

	for (i = 0; i < args->max; ++i) {
		dequeue(args->Q, &j);
		printf("[%d] %d: Received %d\n", args->id, i, j);
		sleep(args->wait);
	}
	
	printf("[%d] finished\n", args->id);

	return NULL;	
}

int main(int argc, char **argv)
{
	unsigned producers, consumers, pwait, cwait, pmax, threads, i, total;
	queue_t Q;
	struct targs *args;
	pthread_t *tids;
	
	producers = argc < 2 ? 1 : atoi(argv[1]);
	consumers = argc < 3 ? 1 : atoi(argv[2]);
	pmax = argc < 4 ? 10 : atoi(argv[3]);
	pwait = argc < 5 ? 1 : atoi(argv[4]);
	cwait = argc < 6 ? 1 : atoi(argv[5]);
	
	threads = producers + consumers;
	total = producers * pmax;
	
	init(&Q);
	
	args = malloc(threads * sizeof(struct targs));
	tids = malloc(threads * sizeof(pthread_t));
	
	for (i = 0; i < consumers; i++) {
		args[i].Q = &Q;
		args[i].id = i;
		args[i].max = (i+1)*total/consumers - i*total/consumers;
		args[i].wait = cwait;
		pthread_create(&tids[i], NULL, consumer, &args[i]);
	}
	for (; i < threads; i++) {
		args[i].Q = &Q;
		args[i].id = i;
		args[i].max = pmax;
		args[i].wait = pwait;
		pthread_create(&tids[i], NULL, producer, &args[i]);
	}
	
	for (i = 0; i < threads; ++i) {
		pthread_join(tids[i], NULL);
	}
	
	destroy(&Q);
	free(args);
	free(tids);
	
	return EXIT_SUCCESS;
}
