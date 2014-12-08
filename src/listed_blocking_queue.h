/*
 * listed_blocking_queue.h
 * Author:MaximusZhou
 */

#ifndef LISTED_BLOCKING_QUEUE_H
#define LISTED_BLOCKING_QUEUE_H

#include "threadpool.h"

typedef struct listed_queue_t listed_queue_t; 

typedef struct queue_node_t{
	struct queue_node_t* next;
	struct queue_node_t* prev;
	task_t task_info;
} queue_node_t;

/* 
 * This function initial listed queue
 *
 *@return On success returns a pointer of a newly listed queue object, on failure NULL is returned
*/
listed_queue_t* queue_init();

/* 
 * This function add a task on the tail of queue
 *
 *@param A pointer of a listed queue object
 *@param A pointer of a queue node object
*/
void append_task(listed_queue_t* queue, queue_node_t* node);

/* 
 * This function remove a task on the head of queue
 *
 *@param A pointer of a listed queue object
 *@return On success returns a pointer of a queue_node_t type object, on failure NULL is returned
*/
queue_node_t* remove_task(listed_queue_t* queue);

/* 
 * This function destory listed queue, release all nodes. All worker threads should be exit while
 * call the function.
 *@param A pointer of a listed queue object
*/
void queue_destory(listed_queue_t* queue);

#endif /* LISTED_BLOCKING_QUEUE_H */

