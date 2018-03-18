#include "smartbell/smartbell_password.h"
#include "smartbell/smartbell_util.h"
#include "resource.h"
#include "log.h"
#include "webutil.h"
#include <string.h>
char init_pw[MAX_PW_LENGTH + 1];
int init_pw_idx = 0;
char input_pw[MAX_PW_LENGTH + 1];
int input_pw_idx = 0;

char getch=NULL;
static int lcd;

void set_password(char* s)
{
	strcpy(init_pw,s);
}

int input_password(){
	getch = NULL;
	resource_read_key_matrix(&getch);

	if(getch != NULL){
		_D("getch : %c", getch);
		switch(getch)
		{
		case 'A':
			break;
		case 'B':
			break;
		case 'C':
			_D("input_pw clear");
			input_pw_idx = 0;
			break;
		case 'D':
			_D("input_pw delete");
			if(input_pw_idx != 0){
				input_pw[--input_pw_idx] ='\0';
				print_lcd(input_pw);
			}
			break;
		case '#':
			if(input_pw_idx == 0)
				print_lcd("PLEASE INPUT PW");
				//_D("please input input_pw");
			else{
				if(strcmp(init_pw, input_pw)){
					print_lcd("CHECK INPUT PW");
					//_D("check input_pw");
					input_pw_idx = 0;
					return 0;
				}
				else{
					print_lcd("UNLOCK");
					return 1;
				}
			}
			break;
		default:
			_D("input_pw input");
			if(input_pw_idx == MAX_PW_LENGTH)
				break;
			input_pw[input_pw_idx++] = getch;
			input_pw[input_pw_idx] = '\0';
			print_lcd(input_pw);
			break;
		}
	}
	return 0;
}
