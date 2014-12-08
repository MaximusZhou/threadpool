/*
 * threadpool.h
 * Author:MaximusZhou
 */

#ifndef THREAD_H
#define THREAD_H

typedef struct threadpool threadpool; 

/* define threadpool error type */
typedef enum {
	ADD_TASK_FAILURE = 1,
} threadpool_error_t;

/* define add routine way */
typedef enum {
	ADD_NOBLOCKING,
	ADD_BLOCKING,
} add_task_t;

/* Is threadpool destorying? */
typedef enum {
	NODESTORYING,
	DESTORY_NOBLOCKING,
	DESTORY_BLOCKING,
} destory_flag_t;

/* define task routine type */
typedef void (*task_routine)(void*);

/* define task type */
typedef struct {
	task_routine routine;
    void *arg;
} task_t;

/*
 * This function creates a threadpool object
 * 
 * @param thread_num The number of worker thread
 * @param max_task_num The max number of task, if value is 0, the imply no number limited
 * @return On success returns a pointer of a newly threadpool object, on failure NULL is returned
 */
threadpool* threadpool_init(unsigned int thread_num, unsigned int max_task_num);

/*
 * This function add a task into queue
 *
 *@param pool The threadpool to which add task
 *@param task_routine The pointer of function will execute task
 *@param arg The argument of task function
 *@param flag If falg is ADD_NOBLOCKING, the function return immediately, or until the queue free
 *@return 0 if all goes well, others value if failure, the failure reason in threadpool_error 
 */
int threadpool_add_task(threadpool* pool, task_routine routine, void* arg, add_task_t flag);

/*
 * This function destory a threadpool object and realse memory
 *
 *@param pool The threadpool is to be realsed
 *@param flag If falg is DESTORY_BLOCKING, then the all woker threads will exit immediately, or 
 *       until all task is done but the thread pool doesn't accept any new tasks
 */
void threadpool_destroy(threadpool* pool, destory_flag_t flag);

#endif /* THREAD_H */
