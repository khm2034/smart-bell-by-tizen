/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * Contact: Jin Yoon <jinny.yoon@samsung.com>
 *          Geunsun Lee <gs86.lee@samsung.com>
 *          Eunyoung Lee <ey928.lee@samsung.com>
 *          Junkyu Han <junkyu.han@samsung.com>
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <glib.h>
#include <Ecore.h>
#include <tizen.h>
#include <service_app.h>
#include "log.h"
#include "resource.h"
#include "connectivity.h"
#include "controller.h"
#include "controller_util.h"
#include "webutil.h"
#include "smartbell.h"

#define CONNECTIVITY_KEY "opened"
#define SENSORING_TIME_INTERVAL 5.0f
#define CAMERA_TIME_INTERVAL 2
#define TEST_CAMERA_SAVE 1
#define CAMERA_ENABLED 1
#define STEP_MOTOR_TIME 14.0f
#define KEY_MATRIX_TIME 0.5f
#define DC_MOTOR_TIME 1.0f

#define NOTE_C 261
#define NOTE_D 294
#define NOTE_E 330
#define NOTE_F 349
#define NOTE_G 392
#define NOTE_A 440
#define NOTE_B 494

typedef struct app_data_s {
	Ecore_Timer *getter_timer;
	connectivity_resource_s *resource_info;
} app_data;

typedef enum {
	SETTING,
	MOVE_ARM,
	RUN_TIMER,
	INPUT_PW,
	RUN_MOTOR,
	STOP_MOTOR,
	MOVE_ARM_EX
}smartbell_state_e;

static smartbell_state_e smartbell_state = SETTING;
static int speed = 1500;
static int cnt = 0;
static Ecore_Timer* music_timer;
//gggfa#
double music[10][2] = {{784, 0.3f},{0, 0.1f}, {784, 0.3f}, {0, 0.1f},
		{784, 0.2f},{0, 0.05f}, {705, 0.1f},{0, 0.05f}, {950, 0.3f}, {0, 0.0f}};
double fail[2][2] = {{785, 1.0f}, {0, 0.0f}};
void play_success()
{
	int i;
	for(i=0; i<10; i++){
		resource_set_passive_buzzer_value(8, music[i][0], music[i][1]);
	}
	ecore_timer_del(music_timer);
}
void play_fail()
{
	int i;
	for(i=0; i<2; i++){
		resource_set_passive_buzzer_value(8, fail[i][0], fail[i][1]);
	}
	ecore_timer_del(music_timer);
}
void setting_timer(float time, Ecore_Task_Cb func, app_data *ad){
	ecore_timer_del(ad->getter_timer);
	ad->getter_timer = ecore_timer_add(time, func, ad);
}
char d[100] = "123";
static Eina_Bool control_sensors_cb(void *data)
{
	app_data *ad = data;
	int ret = -1;
	if(!cnt){
		//smart_bell_socket_connect(d);
		web_util_json_init();
		web_util_json_begin();
		web_util_json_add_string("data", d);
		web_util_json_end();
		web_util_noti_post("115.68.229.127/send_data.php", web_util_get_json_string());
		web_util_json_fini();
	}

#if 0
	switch(smartbell_state){
	case SETTING:
		ret = input_order_num();
		if(ret==1){
			music_timer = ecore_timer_add(0.5f, play_success, NULL);
			//play_success();
			smartbell_state = MOVE_ARM;
			print_lcd("MOVE ARM", 0);
			resource_pca9685_init(4);
			resource_pca9685_set_value_to_channel(4, 0, 0);
		}
		else if(ret == -1)
			music_timer = ecore_timer_add(0.5f, play_fail, NULL);
		break;
	case MOVE_ARM:
		if(cnt == 0){
			setting_timer(STEP_MOTOR_TIME, control_sensors_cb, ad);
			narrow_arm();
			cnt++;
		}
		else{
			stop_arm();
			resource_pca9685_fini(4);
			cnt = 0;
			smartbell_state = RUN_TIMER;
			setting_timer(KEY_MATRIX_TIME, control_sensors_cb, ad);
		}
		break;
	case RUN_TIMER:
		ret = run_timer();
		if(ret == 1){
			cnt = 0;
			smartbell_state = RUN_MOTOR;
			print_lcd("RUN MOTOR", 0);
		}
		else if(ret == 2){
			smartbell_state = INPUT_PW;
			print_lcd("INPUT PW", 0);
		}
		break;
	case INPUT_PW:
		ret = input_password();
		if(ret == 1){
			cnt = 0;
			music_timer = ecore_timer_add(0.5f, play_success, NULL);
			smartbell_state = STOP_MOTOR;
			print_lcd("IDLE_STATE", 0);
			setting_timer(KEY_MATRIX_TIME, control_sensors_cb, ad);
		}
		else if (ret == -1){
			music_timer = ecore_timer_add(0.5f, play_fail, NULL);
			smartbell_state = RUN_TIMER;
		}
		break;
	case RUN_MOTOR: // 벨 누름
		if(cnt==0)
		{
			setting_timer(DC_MOTOR_TIME, control_sensors_cb, ad);
			start_dc_motor(3,-speed);
			cnt++;
		}
		else if( cnt==1)
		{
			start_dc_motor(3,speed);
			cnt++;
		}
		else {
			stop_dc_motor(3);
			smartbell_state = STOP_MOTOR;
			print_lcd("IDLE_STATE", 0);
			setting_timer(KEY_MATRIX_TIME, control_sensors_cb, ad);
		}
		break;
	case STOP_MOTOR:
		ret = input_unlock_password();
		if(ret == 1)
		{
			cnt = 0;
			music_timer = ecore_timer_add(0.5f, play_success, NULL);
			print_lcd("UNLOCK", 0);
			smartbell_state = MOVE_ARM_EX;
		}
		else if(ret == -1)
			music_timer = ecore_timer_add(0.5f, play_fail, NULL);
		break;
	case MOVE_ARM_EX:
		if(cnt == 0){
			resource_pca9685_init(4);
			resource_pca9685_set_value_to_channel(4, 0, 0);
			setting_timer(STEP_MOTOR_TIME, control_sensors_cb, ad);
			extend_arm();
			cnt++;
		}
		else{
			stop_arm();
			resource_pca9685_fini(4);
			cnt = 0;
			smartbell_state = SETTING;
			//print_lcd("IDLE STATE", 0);
			print_lcd("INPUT ORDER NUM", 0);
			setting_timer(KEY_MATRIX_TIME, control_sensors_cb, ad);
		}
		break;
	default:
		break;
	}
#endif

	return ECORE_CALLBACK_RENEW;
}

static bool service_app_create(void *data)
{
	app_data *ad = data;
	int ret = -1;
	const char *path = NULL;

	/**
	 * No modification required!!!
	 * Access only when modifying internal functions.
	 */
	controller_init_internal_functions();

	/**
	 * Create a connectivity resource and registers the resource in server.
	 */
	/*connectivity_set_protocol(CONNECTIVITY_PROTOCOL_HTTP);

	controller_util_get_path(&path);
	if (path == NULL) {
		_E("Failed to get path");
		return false;
	}

	ret = connectivity_set_resource(path, "org.tizen.door", &ad->resource_info);*/
	//if (ret == -1) _E("Cannot broadcast resource");

	/**
	 * Creates a timer to call the given function in the given period of time.
	 * In the control_sensors_cb(), each sensor reads the measured value or writes a specific value to the sensor.
	 */
	//resource_pca9685_init(8);
	//resource_pca9685_fini(8);
	print_lcd("INPUT ORDER NUM", 0);

	//ad->getter_timer = ecore_timer_add(timer_time, control_sensors_cb, ad);
	ad->getter_timer = ecore_timer_add(KEY_MATRIX_TIME, control_sensors_cb, ad);
	if (!ad->getter_timer) {
		_E("Failed to add infrared motion getter timer");
		return false;
	}

	return true;
}

static void service_app_terminate(void *data)
{
	app_data *ad = (app_data *)data;

	if (ad->getter_timer)
		ecore_timer_del(ad->getter_timer);


	/**
	 * Releases the resource about connectivity.
	 */
	connectivity_unset_resource(ad->resource_info);

	/**
	 * No modification required!!!
	 * Access only when modifying internal functions.
	 */
	controller_fini_internal_functions();

	free(ad);
}

static void service_app_control(app_control_h app_control, void *data)
{
	// Todo: add your code here.
}

static void service_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
}

static void service_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void service_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void service_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int main(int argc, char* argv[])
{
	app_data *ad = NULL;
	int ret = 0;
	service_app_lifecycle_callback_s event_callback;
	app_event_handler_h handlers[5] = {NULL, };

	ad = calloc(1, sizeof(app_data));
	retv_if(!ad, -1);

	event_callback.create = service_app_create;
	event_callback.terminate = service_app_terminate;
	event_callback.app_control = service_app_control;

	service_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, service_app_low_battery, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, service_app_low_memory, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, service_app_lang_changed, &ad);
	service_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, service_app_region_changed, &ad);

	ret = service_app_main(argc, argv, &event_callback, ad);

	return ret;
}
