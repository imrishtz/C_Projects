#include <stdio.h> /* REMOVEEEE */ 

#include <stdlib.h> /* putenv getenv */
#include <unistd.h> /* execvpe environ*/
#include <string.h> /* memset */
#include <sys/types.h> /* pid_t pthread_t*/
#include <time.h> /* time difftime */
#include <pthread.h> /* create join */
#include <signal.h> /* sigaction signum */
#include <semaphore.h>
#include <fcntl.h> /* O_CREAT S_IRUSR S_IWUSR */ 

#include "scheduler.h"
#include "wd.h"

#define SNAME_WD "/sem_wd"
#define SNAME_APP "/sem_app"

#define TASK1_INTERVAL 1
#define TASK2_INTERVAL 2
#define TASK3_INTERVAL 9
#define TASK4_INTERVAL 3

#define TIMEOUT 5

enum ret_values
{
	SUCCSESS,
	SIGACT_ERR,
	ALLOCATION_ERR,
	FORK_ERR,
	EXEC_ERR,
	THREAD_CREATE_ERR,
	SEM_ERR
};

/*************************************************************************/
static pid_t g_other_pid;
static volatile sig_atomic_t g_cnt_sig1 = 0;
static int g_term = 0;
static int stop_task2 = 0;
/*************************************************************************/
sem_t *g_sem = NULL;
scheduler_t *g_sched = NULL;
int g_imwd = 0;
/*************************************************************************/
static int Init(struct sigaction *sa1, struct sigaction *sa2, char *argv[]);
static int InitSigaction(struct sigaction *sa1, struct sigaction *sa2);
static void Sig1Handler(int signum);
static void Sig2Handler(int signum);
static int InitSched( char *argv[]);
static int Task1(void *param);
static int Task2(void *param);
static void *RunSched(void *data);
static int Clean(enum ret_values ret);

/* DEBUG */
static int Task3(void *param);
static int Task4(void *param);
/*************************************************************************/
int WDStart(int argc, char *argv[])
{
	pthread_t app_thread;
	enum ret_values ret = 0;
	struct sigaction sa1;
	struct sigaction sa2;
	int is_wd_opened_app = 0;

	g_sem = sem_open(SNAME_WD, O_CREAT, S_IRUSR | S_IWUSR, 0);

	ret = Init(&sa1, &sa2, argv);
	if (SUCCSESS != ret)
	{
		return (ret);
	}
	
	if (1 == g_imwd)
	{
		sem_post(g_sem);
		g_other_pid = getppid();
		RunSched(g_sem);
	}
	else /* i'm APP */
	{
		if (NULL == getenv("is_wd_opened_app"))
		{
			/* first time app opening wd */
			putenv("is_wd_opened_app=1");
			g_other_pid = fork();
			if (0 == g_other_pid)
			{
				if (-1 == execvp("./wd.out", argv))
				{
					return (Clean(EXEC_ERR));
				}
			}
			else if (g_other_pid < 0)
			{
				return (Clean(FORK_ERR));
			}
		}
		else
		{
			g_other_pid = getppid();
		}
		
		sem_post(g_sem);
		
		if (pthread_create(&app_thread, NULL, RunSched, g_sem) != SUCCSESS)
		{
			return (Clean(THREAD_CREATE_ERR));
		}
/*		printf("ps %d\n ", pthread_self());*/
/*		printf("ps %d\n ", app_thread);*/
/*		if (pthread_self() == app_thread)*/
/*		{*/
/*			fprintf(stderr, "thread join self");*/
/*			pthread_join(app_thread, NULL);*/
/*		}*/
	}
	
	return (0);
}

/*************************************************************************/
static int Clean(enum ret_values ret)
{
	SchedulerDestroy(g_sched);
	
	sem_unlink(SNAME_WD);
	sem_unlink(SNAME_APP);
	
	return (ret);
}
/*************************************************************************/
static void *RunSched(void *data)
{
	int num = 0;
	
	while (g_term == 0)
	{
		SchedulerRun(g_sched);
	}
	
	return (NULL);
}
/*************************************************************************/
static int Init(struct sigaction *sa1, struct sigaction *sa2,  char *argv[])
{
	enum ret_values ret = 0;
	
	ret = InitSigaction(sa1, sa2);
	if (SUCCSESS != ret)
	{
		return (ret);
	}
	
	ret = InitSched(argv);
	if (SUCCSESS != ret)
	{
		return (ret);
	}
	
	return (SUCCSESS);
}
/*************************************************************************/
static int InitSigaction(struct sigaction *sa1, struct sigaction *sa2)
{
	int ret = 0;
	
	memset (sa1, 0, sizeof(struct sigaction));
	memset (sa2, 0, sizeof(struct sigaction)); 
	
	sa1->sa_handler = Sig1Handler;
	sa2->sa_handler = Sig2Handler;
	
	ret = sigaction(SIGUSR1, sa1, NULL);
	ret += sigaction(SIGUSR2, sa2, NULL);
	
	if (SUCCSESS != ret)
	{
		return (SIGACT_ERR);
	}
	
	return (SUCCSESS);
}

/*************************************************************************/
static int InitSched(char *argv[])
{
	uuid_t uid_task1;
	uuid_t uid_task2;
	uuid_t uid_task3;
	uuid_t uid_task4;
	
	g_sched = SchedulerCreate();
	if (NULL == g_sched)
	{
		return (ALLOCATION_ERR);
	}
	
	uid_task1 = SchedulerAddTask(g_sched, Task1, NULL, TASK1_INTERVAL);
	if (UUIDIsMatch(uid_task1, UUIDGetInvalidId()))
	{
		return (Clean(ALLOCATION_ERR));
	}
	uid_task2 = SchedulerAddTask(g_sched, Task2, argv, TASK2_INTERVAL);
	if (UUIDIsMatch(uid_task2, UUIDGetInvalidId()))
	{
		return (Clean(ALLOCATION_ERR));
	}
	/* DEBUG */
	if (g_imwd == 0)
	{
		uid_task3 = SchedulerAddTask(g_sched, Task3, argv, TASK3_INTERVAL);
	}
	if (g_imwd == 1)
	{
		uid_task4 = SchedulerAddTask(g_sched, Task3, argv, TASK4_INTERVAL);
	}
	/* END DEBUG */
	return (SUCCSESS);
}
/*************************************************************************/
static int Task1(void *param)
{
	/* relevant to wd only */
	if (1 == g_term) 
	{
		kill(g_other_pid, SIGUSR2);
		SchedulerStop(g_sched);
	}
	
	printf("\nother p = %d ", g_other_pid);
	kill(g_other_pid, SIGUSR1);
	printf("me = %d, my_cnt = %d\n", getpid(), g_cnt_sig1);
	
	return (0);
}
/*************************************************************************/
static int Task3(void *param)
{
	kill(g_other_pid, SIGKILL);
	
	return (1);
}
/*************************************************************************/

static int Task2(void *param)
{
	char **argv = (char **)param;
	char *exec_str = NULL;
	
	exec_str = (g_imwd == 1) ? argv[0]: "./wd.out";  
	if (stop_task2 == 0)
	{
		if (g_cnt_sig1 != 0)
		{
			g_cnt_sig1 = 0;
		}
		else
		{
			SchedulerStop(g_sched);
		
				g_other_pid = fork();
				if (0 == g_other_pid)
				{
					if (-1 == execvp(exec_str, argv))
					{
						return (1);
					}
				}
				
			sem_wait(g_sem);
		}
	}

	return (0);
}
/*************************************************************************/
static void Sig1Handler(int signum)
{
	__sync_add_and_fetch(&g_cnt_sig1, 1);
	
	return;
}
/*************************************************************************/
static void Sig2Handler(int signum)
{
	g_term = 1;
	write(2, "sig2\n", 5);
	
	return;
}
/*************************************************************************/
void WDStop()
{
	time_t start_time;
	time_t current_time;
	
	stop_task2 = 1;
	
	time(&start_time);
	
	do 
	{
		kill(g_other_pid, SIGUSR2);
		sleep(1);
		time(&current_time);
	}
	while (difftime(current_time, start_time) < TIMEOUT && 0 == g_term);
	
	if (0 == g_term)
	{
		kill(g_other_pid, SIGKILL);
	}
	
	SchedulerStop(g_sched);
	
	Clean(SUCCSESS);
	
	return;
}







