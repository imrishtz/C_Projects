#include <unistd.h> 	/* getpid */
#include <sys/types.h> /* pid_t */

#include "uuid.h" 

uuid_t UUIDCreate(void)
{
	uuid_t uuid;
	
	static unsigned long ctr = 0;
		
	++ctr;
	
	uuid.ctr = ctr;
	uuid.pid = getpid();
	gettimeofday(&uuid.time, NULL);
	
	return (uuid);
}
/*******************************************************************/

/* O(1) */
/* Return an invalid uid for error handling */
uuid_t UUIDGetInvalidId(void)
{
	uuid_t uuid;
	
	uuid.ctr = 0;
	uuid.pid = 0;	
	uuid.time.tv_sec = 0;
	uuid.time.tv_usec = 0;	
	
	return (uuid);
}
/*******************************************************************/

/* O(1) */
/* Return 1 if uid1 equals uid2 */
int UUIDIsMatch(uuid_t uid1, uuid_t uid2)
{
	return (uid1.ctr == uid2.ctr &&
			uid1.pid == uid2.pid &&
			uid1.time.tv_sec == uid2.time.tv_sec &&
			uid1.time.tv_usec == uid2.time.tv_usec);
}
