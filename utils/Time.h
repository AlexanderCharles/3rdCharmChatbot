#ifndef _3CCB_UTILS_TIME_
#define _3CCB_UTILS_TIME_

#define _XOPEN_SOURCE 700
#include <time.h>



typedef struct DateTime
{
	/* 9 - Assuming it is always formatted as 'HH:MM:SS' + '\0'. */
	char      serialised_time[9];
	
	/* Bare in mind that if I want to get the current day from
	 * this I need to change the month and year by this amount:
	 * int tm_mday; // day of the month 1-31
	 * int tm_mon;  // month            0-11
	 * int tm_year; // year       minus 1900 */
	struct tm libc_datetime;
} DateTime;



DateTime DateTimeCreateFromTimestamp(char*);
DateTime DateTimeNow                ();
/* (Arg1 - Arg2) */
int      DateTimeDiff               (DateTime, DateTime);



#endif
