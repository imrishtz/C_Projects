#include <stdlib.h> /* malloc, free */
#include <sys/time.h> /* time, time_t*/
#include <assert.h>

#include "task.h" 

struct task_st
{
	uuid_t uid;
	time_t next_run_time; 
	unsigned long interval_in_sec;
	int (*func)(void *param);
	void *param;
};

/*******************************************************************/

task_t *TaskCreate(int (*func)(void *param),
					void *param,
					unsigned long interval_in_sec)
{
	task_t *task = NULL;
	
	assert(func != NULL); 
	
	task = (task_t*)malloc(sizeof(task_t));
	if (NULL == task)
	{
		return (NULL);
	}
	
	task->uid = UUIDCreate();
	task->interval_in_sec = interval_in_sec;
	task->func = func;
	task->param = param;
	task->next_run_time = time(NULL) + interval_in_sec;

	return (task);
}

/*******************************************************************/

/* O(1) */
void TaskDestroy(task_t *task)
{
	assert(task != NULL);
	
	free(task);
	
	return;
}

/*******************************************************************/

/* O(1) */
/* Update next run time of the task */
void TaskUpdate(task_t *task)
{
	assert(task != NULL);
	
	task->next_run_time = time(NULL) + task->interval_in_sec;
	
	return;
}

/*******************************************************************/

/* O(1) */
/* Comparison function
	Return 1 if task1 is before task2 */
int TaskIsBefore(const void *task1, const void *task2, void *param)
{
	assert(task1 != NULL); assert(task2 != NULL);
	
	(void)(param); 
	
	return (((const task_t*)task1)->next_run_time < ((const task_t*)task2)->next_run_time);
}
/*******************************************************************/

/* O(1) */
/* Return the next run time of the task */
time_t TaskGetNextRunTime(const task_t *task)
{
	assert(task != NULL);
	
	return (task->next_run_time);
}
/*******************************************************************/

/* O(1) */
/* Return the task uid */
uuid_t TaskGetID(const task_t *task)
{
	assert(task != NULL);

	return (task->uid);
}
/*******************************************************************/

/* O(1) */
/* Return 1 if task id equals to uid */
int TaskIsMatch(const void *task, const void *uid, void *param)
{
	assert(task != NULL);
	assert(uid != NULL);
	
	(void)(param);
	
	return (UUIDIsMatch(((const task_t*)task)->uid, *(const uuid_t*)uid));
}
/*******************************************************************/

/* O(1) */
/* Run task.
	Return 0 if task should run again after the time interval.
	Return 1 if not required to run again. */
int TaskRun(task_t *task)
{
	assert(task != NULL);
	
	return (task->func(task->param));
}




