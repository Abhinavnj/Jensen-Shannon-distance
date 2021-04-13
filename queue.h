#include <pthread.h>

typedef struct {
	char** data;
	unsigned count;
	unsigned head;
	pthread_mutex_t lock;
} queue_t;

int init(queue_t *Q);
int destroy(queue_t *Q);
int enqueue(queue_t *Q, char* item);
int dequeue(queue_t *Q, char** item);