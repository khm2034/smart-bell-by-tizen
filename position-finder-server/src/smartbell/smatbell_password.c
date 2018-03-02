
#include "smartbell/smartbell_password.h"
#include "resource.h"
#include "log.h"
#include "webutil.h"
char init_pw[MAX_PW_LENGTH];
int init_pw_idx = 0;
char input_pw[MAX_PW_LENGTH];
int input_pw_idx = 0;

char getch=NULL;
int init_password()
{
	getch=NULL;
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
			_D("init_pw clear");
			init_pw_idx = 0;
			break;
		case 'D':
			_D("init_pw delete");
			if(init_pw_idx != 0)
				init_pw_idx--;
			break;
		case '#':
			if(init_pw_idx == 0)
				_D("please input init_pw");
			else{
				_D("set input init_pw");
				web_util_json_init();
				web_util_json_begin();
				web_util_json_add_string("pw", init_pw);
				web_util_json_end();
				web_util_noti_post("115.68.229.127/getData.php", web_util_get_json_string());
				web_util_json_fini();
				return 1;
			}
			break;
		default:
			_D("init_pw input");
			if(init_pw_idx == MAX_PW_LENGTH)
				break;
			init_pw[init_pw_idx++] = getch;
			break;
		}
	}
	return 0;
}

int chk_pw()
{
	int i;
	if(init_pw_idx != input_pw_idx)
		return -1;

	for(i=0; i<init_pw_idx; i++){
		if(init_pw[i] != input_pw[i])
			return -1;
	}

	return 0;
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
			if(input_pw_idx != 0)
				input_pw_idx--;
			break;
		case '#':
			if(input_pw_idx == 0)
				_D("please input input_pw");
			else{
				if(chk_pw()){
					_D("check input_pw");
					input_pw_idx = 0;
					return 0;
				}
				else
					return 1;
			}
			break;
		default:
			_D("input_pw input");
			if(input_pw_idx == MAX_PW_LENGTH)
				break;
			input_pw[input_pw_idx++] = getch;
			break;
		}
	}
	return 0;
}
