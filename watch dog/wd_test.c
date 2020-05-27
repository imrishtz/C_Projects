#include <stdio.h> /**/
#include <stdlib.h>
/* gcc wd.c wd_test.c scheduler.c uuid.c task.c pq.c srt_list.c dlist.c -lpthread -o app.out */
#include "wd.h"
/* THIS IS APP MAIN */
int main(int argc, char *argv[])
{
	int ret = 0;
	size_t i = 0;
	puts("i have opeed");
	
	ret = WDStart(argc, argv);
	if (ret != 0)
	{
		return (ret);
	}

/*	while(1)*/
/*	{}*/
	if (NULL == getenv("first_open_app"))
	{
		putenv("first_open_app=1");
		for (i = 0; i < 25; ++i)
		{
			printf("%d",sleep(1));
		}
	}
	
	WDStop();
	
	return (0);
}
