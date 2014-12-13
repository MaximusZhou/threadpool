/*
 * listed_blocking_queue.c
 * Author:MaximusZhou
 */

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

#include "listed_blocking_queue.h"

struct listed_queue_t {
	queue_node_t* head;
	queue_node_t* tail;

	unsigned int node_num;
	pthread_mutex_t q_lock;
};

listed_queue_t* queue_init()
{
	listed_queue_t* queue = (listed_queue_t*)malloc(sizeof(listed_queue_t));
	if (queue == NULL)
		return NULL;

	queue->head = NULL;
	queue->tail = NULL;
	queue->node_num = 0;

	if (pthread_mutex_init(&(queue->q_lock), NULL) != 0 )
	{
		free(queue);
		return NULL;
	}

	return queue;
}

void append_task(listed_queue_t* queue, queue_node_t* node)
{
	if ((queue == NULL) || (node == NULL))
		return;
	
	pthread_mutex_lock(&queue->q_lock);
	node->next = NULL;
	node->prev = queue->tail;
	if (queue->tail != NULL)
		queue->tail->next = node;
	else
		queue->head = node;  /* list was empty */
	queue->tail = node;
	queue->node_num  = queue->node_num + 1;
	pthread_mutex_unlock(&queue->q_lock);
}

queue_node_t* remove_task(listed_queue_t* queue)
{
	queue_node_t* node; 

	if (queue == NULL)
		return NULL;

	pthread_mutex_lock(&queue->q_lock);
	if (queue->node_num <= 0)
	{
		pthread_mutex_unlock(&queue->q_lock);
		return NULL;
	}
	queue->node_num = queue->node_num - 1;

	node = queue->head;
	queue->head = node->next;
	if (node->next == NULL)
	{/*only a node*/
		assert(queue->node_num == 0);
		queue->tail = NULL;
	}
	else
	{
		node->next->prev = NULL;
	}
	pthread_mutex_unlock(&queue->q_lock);

	return node;
}

void queue_destory(listed_queue_t* queue)
{
	if (queue == NULL)
		return;

	pthread_mutex_lock(&queue->q_lock);
	while (queue->head)
	{
		queue_node_t* temp = queue->head;
		queue->head = temp->next;
		free(temp);
	}
	queue->node_num = 0;
	pthread_mutex_unlock(&queue->q_lock);

	pthread_mutex_destroy(&(queue->q_lock));
	free(queue);
}
