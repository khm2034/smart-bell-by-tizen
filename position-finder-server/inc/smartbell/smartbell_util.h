#ifndef __SMARTBELL_UTIL_H__
#define __SMARTBELL_UTIL_H__

#define TOKEN_COUNT 20
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "resource.h"
#include "log.h"

typedef enum _TOKEN_TYPE {
	TOKEN_STRING,
	TOKEN_NUMBER,
} TOKEN_TYPE;

typedef struct _TOKEN {
	TOKEN_TYPE type;
	union {
		char *string;
		double number;
	};
	bool isArray;
} TOKEN;

typedef struct _JSON {
	TOKEN tokens[TOKEN_COUNT];
} JSON;

void delay_microseconds_hard (unsigned int how_long);
void delay_microseconds (unsigned int how_long);
void delay (unsigned int how_long);
void print_lcd(char* s);
void parse_json(char *doc, int size, JSON *json);
void free_json(JSON *json);
char* get_string_json(JSON *json, char *key);

#endif
