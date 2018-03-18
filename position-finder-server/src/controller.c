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
#define TEST_CAMERA_SAVE 0
#define CAMERA_ENABLED 0

static float timer_time = 0.5f;

typedef struct app_data_s {
	Ecore_Timer *getter_timer;
	connectivity_resource_s *resource_info;
} app_data;

typedef enum {
	SETTING,
	RUN_TIMER,
	INPUT_PW,
	RUN_MOTOR,
	STOP_MOTOR
}smartbell_state_e;

static void __resource_camera_capture_completed_cb(const void *image, unsigned int size, void *user_data)
{
	const char *path = NULL;
	const char *url = NULL;

	controller_util_get_path(&path);

	controller_util_get_image_address(&url);

	web_util_noti_post_image_data(url, path, image, size);

#if TEST_CAMERA_SAVE
	FILE *fp = NULL;
	char *data_path = NULL;
	char file[256];

	data_path = app_get_data_path();

	snprintf(file, sizeof(file), "%sjjoggoba.jpg", data_path);
	free(data_path);
	_D("File : %s", file);

	fp = fopen(file, "w");
	if (!fp) {
		_E("Failed to open file: %s", file);
		return;
	}

	if (fwrite(image, size, 1, fp) != 1) {
		_E("Failed to write image to file");
		return;
	}

	fclose(fp);
#endif
}

double dist = 0;

static void _ultrasonic_sensor_read_cb(double value, void *data)
{
	app_data *ad = data;

#if 1
	if (value < 0) {
		_E("OUT OF RANGE");
	} else {
		_D("Measured Distance : %0.2fcm", value);

//		if (connectivity_notify_double(ad->resource_info, "distance", value) == -1)
//			_E("Cannot notify message");
	}
#endif

	return;
}
static smartbell_state_e smartbell_state = SETTING;
static int speed = 3500;
static int cnt = 1;
static Eina_Bool control_sensors_cb(void *data)
{
	app_data *ad = data;
	int ret = -1;
	switch(smartbell_state){
	case SETTING:
		if(input_order_num()==1){
			smartbell_state = RUN_TIMER;
			print_lcd("RUN TIMER");
		}
		break;
	case RUN_TIMER:
		ret = run_timer();
		if(ret == 1){
			smartbell_state = RUN_MOTOR;
			timer_time = 1.5f;
			print_lcd("RUN MOTOR");
		}
		else if(ret == 2){
			smartbell_state = INPUT_PW;
			print_lcd("INPUT PW");
		}
		break;
	case INPUT_PW:
		if(input_password() == 1){
			smartbell_state = RUN_MOTOR;
			timer_time = 1.5f;
			print_lcd("RUN MOTOR");
		}
		break;
	case RUN_MOTOR:
		if(cnt%5 ==0)
			smartbell_state = STOP_MOTOR;
		else
			start_dc_motor(3,speed);
		cnt++;
		break;
	case STOP_MOTOR:
		stop_dc_motor(3);
		speed = -speed;
		smartbell_state = RUN_MOTOR;
		break;
	}


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
	print_lcd("INPUT ORDER NUM");
	ad->getter_timer = ecore_timer_add(timer_time, control_sensors_cb, ad);
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
