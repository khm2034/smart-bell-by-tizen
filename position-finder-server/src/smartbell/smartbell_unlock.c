#include<string.h>
#include"smartbell.h"
#include "resource.h"
#include "log.h"
#include "webutil.h"
char unlock_pw[MAX_UNLOCK_LENGTH + 1];
int unlock_pw_idx = 0;
static char getch=NULL;
static flag;
size_t post_unlock_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t res_size = 0;
	JSON json = {0,};
	res_size = size*nmemb;

	if (res_size > 0)
	{
		_I("POST response : %s", ptr);
		parse_json(ptr, strlen(ptr), &json);
		if(strcmp(get_string_json(&json, "success"), "1") == 0){
			flag = 1;
		}
		else
			flag = 0;
	}

	return res_size;
}

int input_unlock_password()
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
			unlock_pw_idx = 0;

			break;
		case 'D':
			_D("init_pw delete");
			if(unlock_pw_idx != 0){
				unlock_pw[--unlock_pw_idx] ='\0';
				print_lcd(unlock_pw, 0);
			}
			break;
		case '#':
			if(unlock_pw_idx == 0)
				print_lcd("PLEASE INPUT UNLOCK PW", 0);
			else{
				print_lcd("", 0);
				web_util_json_init();
				web_util_json_begin();
				web_util_json_add_string("UNLOCK", unlock_pw);
				web_util_json_end();
				custom_web_util_noti_post("115.68.229.127/checkUnlock.php", web_util_get_json_string(), post_unlock_callback);
				web_util_json_fini();
				unlock_pw_idx = 0;
				if(flag)
					return 1;
				else{
					unlock_pw_idx = 0;
					print_lcd("CHECK INPUT UNLOCK PW", 0);
					return -1;
				}
			}
			break;
		default:
			if(unlock_pw_idx == MAX_UNLOCK_LENGTH)
				break;
			unlock_pw[unlock_pw_idx++] = getch;
			unlock_pw[unlock_pw_idx] = '\0';
			print_lcd(unlock_pw, 0);
			break;
		}
	}
	return 0;
}
