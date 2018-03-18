#include "smartbell/smartbell_util.h"
#include "resource.h"
#include "log.h"
#include <string.h>
#include <time.h>

static int order_time;
static int cur_time;
static time_t timer;
static struct tm *t;
static char getch;
void set_time(char* s)
{
	_D("set time : %s", s);
	for(int i=0; s[i]; i++)
	{
		if(s[i] == ':')
			continue;
		order_time = order_time*10 + s[i]-'0';
	}
}

int run_timer()
{
	getch = NULL;
	resource_read_key_matrix(&getch);

	if(getch != NULL){
		_D("getch : %c", getch);
		switch(getch)
		{
		case 'A':
		case 'B':
		case 'C':
		case 'D':
			break;
		case '#':
			return 2;
		}
	}

	timer = time(NULL);
	t = localtime(&timer);
	_D("order : %d", order_time);
	cur_time = 0;
	cur_time = t->tm_hour*10000 + t->tm_min*100 + t->tm_sec;
	_D("cur : %d", cur_time);
	if(cur_time >= order_time)
		return 1;
	return 0;
}
