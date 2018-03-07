#include "smartbell/smartbell_util.h"

void delay_microseconds_hard (unsigned int how_long)
{
	struct timeval tNow, tLong, tEnd ;

	gettimeofday (&tNow, NULL) ;
	tLong.tv_sec  = how_long / 1000000 ;
	tLong.tv_usec = how_long % 1000000 ;
	timeradd (&tNow, &tLong, &tEnd) ;

	while (timercmp (&tNow, &tEnd, <))
		gettimeofday (&tNow, NULL) ;
}

void delay_microseconds (unsigned int how_long)
{
	struct timespec sleeper ;
	unsigned int uSecs = how_long % 1000000 ;
	unsigned int wSecs = how_long / 1000000 ;

	if (how_long ==   0)
		return ;
	else if (how_long  < 100)
		delay_microseconds_hard (how_long) ;
	else
	{
		sleeper.tv_sec  = wSecs ;
		sleeper.tv_nsec = (long)(uSecs * 1000L) ;
		nanosleep (&sleeper, NULL) ;
	}
}
void delay (unsigned int how_long){
	struct timespec sleeper, dummy ;

	sleeper.tv_sec  = (time_t)(how_long / 1000) ;
	sleeper.tv_nsec = (long)(how_long % 1000) * 1000000 ;

	nanosleep (&sleeper, &dummy) ;
}
