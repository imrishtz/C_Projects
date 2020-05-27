#include <stdio.h>
#include "wd.h"
#include <pthread.h> /* join */

extern scheduler_t *g_sched;
extern int g_imwd;

/* THIS IS WD MAIN */
int main(int argc, char *argv[])
{
	g_imwd = 1;
	

	WDStart(argc, argv);
	puts("dsa");
	SchedulerDestroy(g_sched);
	
	
	
	return (0);
}
