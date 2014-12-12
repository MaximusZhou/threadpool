/*
* test_tp_main.c
 * Author:MaximusZhou
* $gcc -g  -lpthread -I src/ -o test/test_tp_main  src/listed_blocking_queue.c src/threadpool.c test/test_tp_main.c
* $for((i=1;i<=10000;i++));do test/test_tp_main;done
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

static void test_add(unsigned int test_no,destory_flag_t destory_type);

int main(int argc, char **argv)
{
	pthread_mutex_init(&lock, NULL);

	test_add(0, DESTORY_NOBLOCKING);
	usleep(10);
	test_add(1, DESTORY_BLOCKING);

	pthread_mutex_destroy(&lock);

	return 0;
}

void dummy_task(void *arg) {
	usleep(10);
	pthread_mutex_lock(&lock);
	done++;
	pthread_mutex_unlock(&lock);
}


static void test_add(unsigned int test_no, destory_flag_t destory_type)
{
	int tasks = 0;
	add_task_t add_type;
	threadpool *pool;
	done = 0;

	assert((pool = threadpool_init(THREAD, QUEUE)) != NULL);
	printf("Test NO %d: Pool started with %d threads and queue size of %d\n", test_no, THREAD, QUEUE);

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
	printf("Test NO %d: Added %d tasks\n", test_no, tasks);

	while((tasks / 2) > done) {
		usleep(10);
	} 
	threadpool_destroy(pool, destory_type); 
	printf("Test NO %d: Did %d tasks\n", test_no, done);

	if(destory_type == DESTORY_BLOCKING)
		assert(tasks == done);
}
