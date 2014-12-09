/*
*gcc -g  -lpthread -I src/ -o test/test_tp_main  src/listed_blocking_queue.c src/threadpool.c test/test_tp_main.c
*/
#define THREAD 5
#define QUEUE  10

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"

int tasks = 0, done = 0;
pthread_mutex_t lock;

void dummy_task(void *arg) {
	usleep(10);
	pthread_mutex_lock(&lock);
	done++;
	pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
	threadpool *pool;

	pthread_mutex_init(&lock, NULL);

	assert((pool = threadpool_init(THREAD, QUEUE)) != NULL);
	fprintf(stderr, "Pool started with %d threads and "
			"queue size of %d\n", THREAD, QUEUE);

	while(threadpool_add_task(pool, &dummy_task, NULL, ADD_NOBLOCKING) == 0) {
		pthread_mutex_lock(&lock);
		tasks++;
		pthread_mutex_unlock(&lock);
	}   

	fprintf(stderr, "Added %d tasks\n", tasks);

	while((tasks / 2) > done) {
		usleep(10);
	}   
	threadpool_destroy(pool, 0); 
	fprintf(stderr, "Did %d tasks\n", done);

	return 0;
}
