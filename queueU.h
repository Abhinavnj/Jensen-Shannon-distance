#include <pthread.h>

typedef struct {
	char** data;
	unsigned count;
	unsigned head;
	pthread_mutex_t lock;
    pthread_cond_t read_ready;
} queueU_t;

int initU(queueU_t *Q);
int destroyU(queueU_t *Q);
int enqueueU(queueU_t *Q, char* item);
int dequeueU(queueU_t *Q, char** item);
void printU(queueU_t* Q);