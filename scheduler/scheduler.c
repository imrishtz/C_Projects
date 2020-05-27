#include <stdlib.h> 	/* malloc, free */
#include <unistd.h> 	/* sleep */
#include <sys/time.h> 	/* time , time_t*/
#include <assert.h>

#include "pq.h"
#include "task.h"
#include "scheduler.h"

enum {
	STOP = 0,
	RUN = 1
	};

struct  scheduler_st
{
	pq_t *pq;
	int is_sched_run;
};

/*******************************************************************/

/* return NULL if fails */
scheduler_t *SchedulerCreate(void)
{
	scheduler_t *sched =(scheduler_t*)malloc(sizeof(scheduler_t));
	if (NULL == sched)
	{
		return (NULL);
	}

	sched->pq = PQCreate(TaskIsBefore, NULL);		
	if (NULL == sched->pq)
	{
		free(sched);
		
		return (NULL);
	}
	
	sched->is_sched_run = RUN;
	
	return (sched);
}
/*******************************************************************/

/* O(n) */
void SchedulerDestroy(scheduler_t *sched)
{
	assert(sched);
	
	/* free node's data */
	SchedulerClear(sched);
	/* free nodes */
	PQDestroy(sched->pq); sched->pq = NULL;

	free(sched); sched = NULL;
	
	return;
}
/*******************************************************************/

/* O(1) */
int SchedulerIsEmpty(const scheduler_t *sched)
{
	assert(sched);
	
	return (PQIsempty(sched->pq));
}
/*******************************************************************/

/* O(n) */
size_t SchedulerSize(const scheduler_t *sched)
{
	assert(sched);
	
	return (PQSize(sched->pq));
}
/*******************************************************************/

/* O(n) */
/* Add a task to the scheduler and return the task id */
uuid_t SchedulerAddTask(scheduler_t *sched,
			int (*func)(void *param),
			void *param,
			unsigned long interval_in_sec)
{
	task_t *new_task = NULL;
	
	assert(sched);
	assert(func);
	
	new_task = TaskCreate(func, param, interval_in_sec);
	if (NULL == new_task)
	{
		return (UUIDGetInvalidId());		
	}
	
	if (PQEnqueue(sched->pq, new_task))
	{
		return (UUIDGetInvalidId());
	}
	
	return (TaskGetID(new_task));	
}
/*******************************************************************/

/* O(n) */
/* Remove a specific task from the scheduler, return 1 if didnt find */
int SchedulerRemove(scheduler_t *sched, uuid_t id)
{
	task_t *erased_task = NULL;

	assert(sched);
	
	erased_task = (task_t*)PQErase(sched->pq, TaskIsMatch, &id, NULL);
	if (NULL ==  erased_task)
	{
		return (1);
	}
	
	TaskDestroy(erased_task);
	
	return (0);
}
/*******************************************************************/

/* Run the scheduler */
void SchedulerRun(scheduler_t *sched)
{
	task_t *task = NULL;
	time_t next_run_time;
	
	assert(sched);


	sched->is_sched_run = RUN;
	
	while ((!SchedulerIsEmpty(sched)) && (RUN == sched->is_sched_run))
	{
		task = PQDequeue(sched->pq);
		
		next_run_time = TaskGetNextRunTime(task);
	
		while (time(NULL) < next_run_time)
		{
			sleep(next_run_time - time(NULL));
		}
		/* run task */
		/* if fails or not going back to list */
		if (1 == TaskRun(task))
		{
			TaskDestroy(task); task = NULL;
			
			continue;				
		}

		TaskUpdate(task);

		/* if enqueue fails, freeing task */
		if (PQEnqueue(sched->pq, task))
		{
			TaskDestroy(task); task = NULL;
		}
		
	}
	
	return;
}
/*******************************************************************/

/* O(1) */
/* Stop the scheduler */
void SchedulerStop(scheduler_t *sched)
{
	assert(sched);
	
	sched->is_sched_run = STOP;
		
	return;	
}
/*******************************************************************/

/* O(n) */
/* Clear all tasks from scheduler */
void SchedulerClear(scheduler_t *sched)
{
	assert(sched);
	
	while (!SchedulerIsEmpty(sched))
	{
		TaskDestroy(PQDequeue(sched->pq));
	}
		
	return;
}
