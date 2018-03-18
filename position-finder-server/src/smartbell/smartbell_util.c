#include "smartbell/smartbell_util.h"

static int lcd;

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

void print_lcd(char* s){
	resource_close_key_matrix();
	lcd = resource_1602A_LCD_init(2, 16, 8, LCD_RS, LCD_E, LCD_D0, LCD_D1, LCD_D2
			, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
	resource_1602A_LCD_position(lcd, 0, 0);
	if(strlen(s) <= 16)
		resource_1602A_LCD_puts(lcd, s);
	else {
		char s1[17]={0,}; memcpy(s1, s, 16);
		char s2[17]={0,}; memcpy(s2, s+16, strlen(s)-16);
		resource_1602A_LCD_position(lcd, 0, 0);
		resource_1602A_LCD_puts(lcd, s1);
		resource_1602A_LCD_position(lcd, 0, 1);
		resource_1602A_LCD_puts(lcd, s2);
	}
	resource_1602A_LCD_close();
}

void parse_json(char *doc, int size, JSON *json){
	int tokenIndex = 0;
	int pos = 0;

	if (doc[pos] != '{')
		return;

	pos++;

	while (pos < size){
		switch (doc[pos]){
			case '"':{
				char *begin = doc + pos + 1;
				char *end = strchr(begin, '"');
				int stringLength = end - begin;
				json->tokens[tokenIndex].type = TOKEN_STRING;
				json->tokens[tokenIndex].string = malloc(stringLength + 1);
				memset(json->tokens[tokenIndex].string, 0, stringLength + 1);
				memcpy(json->tokens[tokenIndex].string, begin, stringLength);
				tokenIndex++;
				pos = pos + stringLength + 1;
			}
			break;
		}
		pos++;
	}
}

void free_json(JSON *json){
	for (int i = 0; i < TOKEN_COUNT; i++){
		if (json->tokens[i].type == TOKEN_STRING)
			free(json->tokens[i].string);
	}
}

char* get_string_json(JSON *json, char *key){
	for (int i = 0; i < TOKEN_COUNT; i++){
		if (json->tokens[i].type == TOKEN_STRING &&
			strcmp(json->tokens[i].string, key) == 0){
			if (json->tokens[i + 1].type == TOKEN_STRING)
				return json->tokens[i + 1].string;
		}
	}
	return NULL;
}
