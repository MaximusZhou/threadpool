/*
* test_tp_main.c
 * Author:MaximusZhou
* gcc -g  -lpthread -I src/ -o test/test_tp_main  src/listed_blocking_queue.c src/threadpool.c test/test_tp_main.c
*/
#define THREAD 5
#define QUEUE  10
#define MAX_THREAD  1000

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"

int done = 0;
pthread_mutex_t lock;

void dummy_task(void *arg) {
	usleep(10);
	pthread_mutex_lock(&lock);
	done++;
	pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
	int tasks = 0;
	add_task_t add_type;
	threadpool *pool;

	pthread_mutex_init(&lock, NULL);

	assert((pool = threadpool_init(THREAD, QUEUE)) != NULL);
	printf("Pool started with %d threads and queue size of %d\n", THREAD, QUEUE);

	while(1) 
	{
		if(tasks % 2 == 0)
			add_type = ADD_NOBLOCKING;
		else
			add_type = ADD_BLOCKING;

		if (threadpool_add_task(pool, &dummy_task, NULL, add_type) != 0)
			break;
		
		tasks++;
		if(tasks >= MAX_THREAD)
			break;
	} 
	printf("Added %d tasks\n", tasks);

	while((tasks / 2) > done) {
		usleep(10);
	} 
	threadpool_destroy(pool, 0); 
	printf("Did %d tasks\n", done);

	pthread_mutex_destroy(&lock);

	return 0;
}
