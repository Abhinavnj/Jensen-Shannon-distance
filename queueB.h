#include <pthread.h>

#ifndef QSIZE
#define QSIZE 8
#endif

typedef struct {
	char* data[QSIZE];
	unsigned count;
	unsigned head;
	int open;
	pthread_mutex_t lock;
	pthread_cond_t read_ready;
	pthread_cond_t write_ready;
} queueB_t;

int initB(queueB_t *Q);
int destroyB(queueB_t *Q);
int enqueueB(queueB_t *Q, char* item);
int dequeueB(queueB_t *Q, char** item);
int qcloseB(queueB_t *Q);
void printB(queueB_t* Q);