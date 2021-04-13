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