#include<string.h>
#include"smartbell.h"
#include "resource.h"
#include "log.h"
#include "webutil.h"
char order_num[MAX_ORDER_NUM_LENGTH + 1];
int order_num_idx = 0;
char callback_message[100];
static char getch=NULL;
static flag;
size_t post_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
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
			set_password(get_string9_json(&json, "password"));
			set_time(get_string_json(&json, "btime"));
		}
		else
			flag = 0;
	}

	return res_size;
}

int input_order_num()
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
			order_num_idx = 0;

			break;
		case 'D':
			_D("init_pw delete");
			if(order_num_idx != 0){
				order_num[--order_num_idx] ='\0';
				print_lcd(order_num);
			}
			break;
		case '#':
			if(order_num_idx == 0)
				print_lcd("PLEASE INPUT PW");
			else{
				_D("set input init_pw");
				print_lcd("");
				web_util_json_init();
				web_util_json_begin();
				web_util_json_add_string("orderNum", order_num);
				web_util_json_end();
				custom_web_util_noti_post("115.68.229.127/getOrderInfo.php", web_util_get_json_string(), post_callback);
				web_util_json_fini();
				order_num_idx = 0;
				if(flag)
					return 1;
				else
					print_lcd("CHECK INPUT ORDER NUMBER");
			}
			break;
		default:
			_D("init_pw input");
			if(order_num_idx == MAX_ORDER_NUM_LENGTH)
				break;
			order_num[order_num_idx++] = getch;
			order_num[order_num_idx] = '\0';
			print_lcd(order_num);
			break;
		}
	}
	return 0;
}
