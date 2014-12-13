Features
=========================================
	* A thread pool implementation by C language
	* It is very simply to use the thread pool, just need to include threadpool.h in your code
	* Start all threads on creation of the thread pool
	* Adding task can be in blocking way or noblocking way
	* Destory thread pool can be in wait_for_active_tasks way or immediately way
Future Work
=========================================
	* Change dynamicly the size of thread pool
	* Policy-based thread pool 
	* Add more destory thread pool policy
Test
=========================================
	$make
	$./test/test_tp_main
	$sh test/repeat_run_test_tp_main.sh
