#include "Time.h"

#include <stdio.h>
#include <string.h>



/* 
 * Expects i_timestamp to be HH:MM:SS.
 * */
DateTime
DateTimeCreateFromTimestamp(char* i_timestamp)
{
	DateTime result = { 0 };
	time_t now;
	
	strncpy(result.serialised_time, i_timestamp, 8);
	
	/* Get date (and time) */
	now = time(0);
	result.libc_datetime = *localtime(&now);
	/* Overwrite the time */
	sscanf(result.serialised_time, "%d:%d:%d",
	       &result.libc_datetime.tm_hour,
	       &result.libc_datetime.tm_min,
	       &result.libc_datetime.tm_sec);
	
	return(result);
}

DateTime
DateTimeNow()
{
	DateTime result = { 0 };
	time_t now;
	
	now = time(0);
	result.libc_datetime = *localtime(&now);
	strftime(result.serialised_time, 9, "%H:%M:%S", &result.libc_datetime);
	
	return(result);
}

/* I believe this is down to the tenth of a second. (So 1 = 100ms). */
int
DateTimeDiff(DateTime i_a, DateTime i_b)
{
	return((int) difftime(mktime(&i_a.libc_datetime),
	                      mktime(&i_b.libc_datetime)));
}
