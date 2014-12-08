/*
 * threadpool.c
 * Author:MaximusZhou
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

#include "threadpool.h"
#include "listed_blocking_queue.h"

struct threadpool {
	listed_queue_t *task_queue;
	unsigned int   task_num;
	unsigned int   max_task_num;
	unsigned int   wait_tnum_on_add_task; /*the number of thread while add task in queue*/

	pthread_t        *threads;
	pthread_mutex_t  lock;
	pthread_cond_t   del_task_cond;
	pthread_cond_t   add_task_cond;
	pthread_cond_t   wait_thread_cond;

	unsigned int thread_num;

	destory_flag_t destory_flag; 
};

static void* thread_func(void *arg);

threadpool* threadpool_init(unsigned int thread_num,unsigned int max_task_num)
{
	threadpool *pool;	
	int i;

	if ((pool = (threadpool *)malloc(sizeof(threadpool))) == NULL)
		return NULL;

	/* Initialize */
	pool->thread_num = 0;
	pool->destory_flag = NODESTORYING;

	pool->task_num = 0;
	pool->max_task_num = max_task_num;
	pool->task_queue = queue_init();
	if (pool->task_queue == NULL)
	{
		free(pool);
		return NULL;
	}

	pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);
	if (pool->threads == NULL)
	{
		queue_destory(pool->task_queue);
		free(pool);
		return NULL;
	}

	if (pthread_mutex_init(&(pool->lock), NULL) != 0)
	{
		queue_destory(pool->task_queue);
		free(pool);
		free(pool->threads);
		return NULL;
	}

	if (pthread_cond_init(&(pool->del_task_cond),NULL) != 0)
	{
		queue_destory(pool->task_queue);
		free(pool);
		free(pool->threads);
		pthread_mutex_destroy(&(pool->lock));
		return NULL;
	}

	if (pthread_cond_init(&(pool->add_task_cond),NULL) != 0)
	{
		queue_destory(pool->task_queue);
		free(pool);
		free(pool->threads);
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->del_task_cond));
		return NULL;
	}

	if (pthread_cond_init(&(pool->wait_thread_cond),NULL) != 0)
	{
		queue_destory(pool->task_queue);
		free(pool);
		free(pool->threads);
		pthread_mutex_destroy(&(pool->lock));
		pthread_cond_destroy(&(pool->del_task_cond));
		pthread_cond_destroy(&(pool->add_task_cond));
		return NULL;
	}

	/* Create worker thread */
	for(i = 0; i < thread_num; i ++)
	{
		int res = pthread_create(&(pool->threads[i]), NULL, thread_func, pool);
		if(res != 0)
		{
			destory_flag_t exit_code = DESTORY_NOBLOCKING;
			threadpool_destroy(pool, exit_code);
			return NULL;
		}
		pool->thread_num = pool->thread_num + 1;
	}

	return pool;
}

static void* thread_func(void *arg)
{
	threadpool *pool = (threadpool*)arg;
	task_t  task_object;
	queue_node_t *task_node;
	for(;;)
	{
		pthread_mutex_lock(&(pool->lock));
		/*Wait on conditon variable, check for task number and destory flag*/
		while((pool->task_num <= 0) && (pool->destory_flag == NODESTORYING))
		{
			pthread_cond_wait(&(pool->del_task_cond), &(pool->lock));
		}

		if((pool->destory_flag == DESTORY_NOBLOCKING) || 
			((pool->destory_flag == DESTORY_BLOCKING) && (pool->task_num <= 0)))
			break;

		/*get task from task queue*/
		task_node = remove_task(pool->task_queue);
		if(task_node == NULL)
			continue;

		task_object = task_node->task_info;
		pool->task_num = pool->task_num - 1;
		free(task_node);

		pthread_cond_signal(&pool->add_task_cond);
		pthread_mutex_unlock(&(pool->lock));

		(*(task_object.routine))(task_object.arg);
	}
	
	pthread_mutex_unlock(&(pool->lock));
	pthread_exit(NULL);

	return (void*)0;
}

int threadpool_add_task(threadpool* pool, task_routine routine, void* arg, add_task_t flag)
{
	if((pool == NULL) || (routine == NULL) || 
		((flag != ADD_BLOCKING) && (flag != ADD_NOBLOCKING)))
		return ADD_TASK_FAILURE;

	fprintf(stderr, "0");
	pthread_mutex_lock(&(pool->lock));

	/*if the call is no blocking add task and the task queue is full, then return immediately*/
	if((flag == ADD_NOBLOCKING) && (pool->task_num >= pool->max_task_num))
	{
		pthread_mutex_unlock(&(pool->lock));
		return ADD_TASK_FAILURE;
	}

	fprintf(stderr, "1");
	pool->wait_tnum_on_add_task = pool->wait_tnum_on_add_task + 1;
	while((pool->task_num >= pool->max_task_num) && (pool->destory_flag == NODESTORYING))
	{
		if(pthread_cond_wait(&(pool->add_task_cond), &(pool->lock)) != 0)
		{
			pool->wait_tnum_on_add_task = pool->wait_tnum_on_add_task - 1;
			return ADD_TASK_FAILURE;
		}
	}
	pool->wait_tnum_on_add_task = pool->wait_tnum_on_add_task - 1;

	fprintf(stderr, "2");
	if(pool->destory_flag != NODESTORYING)
	{
		pool->wait_tnum_on_add_task = 0;
		pthread_cond_signal(&pool->wait_thread_cond);
		pthread_mutex_unlock(&(pool->lock));
		return ADD_TASK_FAILURE;
	}
	fprintf(stderr, "3");
	queue_node_t *node = (queue_node_t*)malloc(sizeof(queue_node_t));
	if(node == NULL)
		return ADD_TASK_FAILURE;
	(node->task_info).routine = routine;
	(node->task_info).arg = arg;
	append_task(pool->task_queue, node);
	fprintf(stderr, "4");
	
	pool->task_num = pool->task_num + 1;
	pthread_cond_signal(&pool->del_task_cond);
	pthread_mutex_unlock(&pool->lock);
	fprintf(stderr, "5");

	return 0;
}

void threadpool_destroy(threadpool* pool, destory_flag_t flag)
{
	int i;
	if((pool == NULL) || 
		(pool->destory_flag != NODESTORYING) ||
		((flag != DESTORY_BLOCKING) && (flag != DESTORY_NOBLOCKING)))
		return;

	pool->destory_flag = flag;
	pthread_cond_broadcast(&pool->add_task_cond);
	pthread_mutex_lock(&pool->lock);
	while(pool->wait_tnum_on_add_task > 1)
		pthread_cond_wait(&pool->wait_thread_cond, &pool->lock);

	pthread_mutex_unlock(&pool->lock);
	pthread_cond_broadcast(&pool->del_task_cond);

	for(i = 0; i < pool->thread_num; i++)
	{
		pthread_join(pool->threads[i],NULL);
	}
	
	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->del_task_cond);
	pthread_cond_destroy(&pool->add_task_cond);
	pthread_cond_destroy(&pool->wait_thread_cond);

	queue_destory(pool->task_queue);
	free(pool->threads);
	free(pool);
}
