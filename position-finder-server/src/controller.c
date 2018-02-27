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

#define CONNECTIVITY_KEY "opened"
#define SENSORING_TIME_INTERVAL 5.0f
#define CAMERA_TIME_INTERVAL 2
#define TEST_CAMERA_SAVE 0
#define CAMERA_ENABLED 0

char init_pw[8];
char input_pw[8];
int init_index = 0;
int input_index = 0;
int error_count=0;
int state = 0;

typedef struct app_data_s {
	Ecore_Timer *getter_timer;
	connectivity_resource_s *resource_info;
} app_data;

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

static void setDist(double distance, void *data)
{
	_D("set Dist : %lf", distance);
	dist = distance;
}

static int cycle = 0;

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
static int cnt = 0;

static Eina_Bool control_sensors_cb(void *data)
{
	app_data *ad = data;
	uint32_t value = -1;
	int ret = -1;

#if 0
	ret = resource_read_ultrasonic_sensor(20, 21, _ultrasonic_sensor_read_cb, NULL);
	if (ret != 0) _E("Cannot read sensor value");

	_D("Detected distance : %lf", dist);
#endif


#if CAMERA_ENABLED
	static unsigned int count = 0;

	if (count % CAMERA_TIME_INTERVAL == 0) {
		ret = resource_capture_camera(__resource_camera_capture_completed_cb, NULL);
		if (ret < 0)
			_E("Failed to capture camera");
	}

	count++;
#endif

#if 0 // servo motor
	// GND, OE : ground , SCR : 5pin, SDA : 3pin,  vcc v+ : 5v
	_D("cycle : %d", cycle);
	switch(cycle)
	{
	case 0:
		++cycle;
		// channel = 1, on;
		ret = resource_set_servo_motor_value(1, 1);
		break;
	case 1:
		++cycle;
		ret = resource_set_servo_motor_value(1, 0);
		break;
	case 2:
		++cycle;
		// channel = 1, off
		//ret = resource_set_servo_motor_value(1, MOTOR_STATE_BACKWARD);
		break;
	case 3:
		cycle = 0;
		//ret = resource_set_servo_motor_value(1, MOTOR_STATE_STOP);
		break;
	}


	if (ret != 0) _E("Cannot read sensor value");

	//_D("Detected distance : %lf", dist);
#endif

//#if 1 // steoper motor
//	// GND, OE : ground , SCR : 5pin, SDA : 3pin,  vcc v+ : 5v
//	if(cnt % 5 == 0)
//		ret = resource_set_stepper_motor_driver_L298N_speed(0, 0);
//	else
//		ret = resource_set_stepper_motor_driver_L298N_speed(0, 1000);
//	cnt++;
//
//	if (ret != 0) _E("Cannot read sensor value");
//
//	//_D("Detected distance : %lf", dist);
//#endif

#if 0 //led
    //ret = resource_write_led(21,1);
    //_D("Turn on the led");
      ret = resource_write_led(4,1);
    _D("Turn off the led");
    if(ret!=0)
    {
        _E("Cannot read led");
    }
#endif

#if 0 //buzzer
      ret = resource_write_buzzer(4,1);
    _D("Turn off the buzzer");
    if(ret!=0)
    {
        _E("Cannot read buzzer");
    }
#endif

#if 0 //touch
    resource_write_led(21,0);
    resource_write_buzzer(4,0);
    ret = resource_read_touch_sensor(17 , &value);
    ret2 = resource_read_touch_sensor(24 , &value2);
    /*if(ret != 0 || ret2 != 0)
        {
            _E("Cannot read flame");
            resource_write_led(8,0);
        }*/
    if (value == 0){
    	_D("Turn on the led by touching");
    	resource_write_led(21,1);
    }
    else if(value2 == 0)
    {
    	resource_write_buzzer(4,1);
    }
    else{
        _D("Turn off the led by no touching");
        resource_write_led(21,0);
        resource_write_buzzer(4,0);
    }
#endif

#if 0 //flame
    _D("reading flame sensor value");
    ret = resource_read_flame_sensor(20, &value);
    if(ret != 0)
    {
        _E("Cannot read flame");
        //ret = resource_write_led(21,0);
    }
    else{
        _D("Turn on the led");
        ret = resource_write_led(21,1);
        _D("Turn off the led");
        ret = resource_write_led(21,0);


        /*if(position < .5)
        {
            _D("Turn on the led");
            ret = resource_write_led(21,1);
            return ECORE_CALLBACK_RENEW;
        }
        _D("Turn off the led");
        ret = resource_write_led(21,0);
        return ECORE_CALLBACK_CANCEL;
    }
    ecore_animator_timeline_add(5.,_control_sensors_cb,0);*/
#endif

#if 0 //vibe
    resource_read_vibration_sensor(20, &value);
    if(value == 0)
    {
        _E("no vibe");
        _D("Turn off the led");
        ret = resource_write_led(21,0);
        if(connectivity_notify_double(ad->resource_info,"distance",value)==-1)
                    _E("Cannot notify message");
    }
    else
    {
        _D("vibe");
        _D("Turn on the led");
        ret = resource_write_led(21,1);
        if(connectivity_notify_double(ad->resource_info,"vibe",value)==-1)
                    _E("Cannot notify message");
    }

#endif

#if 1
	char getch=NULL;
	resource_write_led(4,0);
	resource_write_buzzer(24,0);
	//_D("read key matrix");
	resource_read_key_matrix(&getch);
	if(getch != NULL){
		switch(getch)
		{
		case 'A':
			if(init_pw[0]!=NULL){
				_D(" [Set entering value as password] ");
				state = 1;
				resource_write_led(4,1);
			}
			else
			{
				_E("Must enter Init password");
			}
			break;
		case 'B':
			if(state==0)
			{
				if(init_index>0){
					_D("[Delete] previous initial value ");
					init_pw[init_index-1]=NULL;
					init_index--;
				}
				else {
					_E("This command can't be executed");
				}
			}
			else if(state==1)
			{
				if(input_index>0)
				{
					_D("[Delete] previous input value");
					input_pw[input_index-1]=NULL;
					input_index--;
				}
				else
				{
					_E("This command can't be executed");
				}
			}
			break;
		case 'C':
			clear:
			if(state==0)
			{
				_D("[Reset] Initial password is entering reset");
				init_pw[0]='\0';
				init_index=0;
			}
			else if(state==1)
			{
				_D("[Reset] Input password is entering reset");
				input_pw[0]='\0';
				input_index=0;
			}
			break;
		case 'D':
			_D("[ Display Now values]");
			_D("[ Now init value : %s ]",init_pw);
			_D("[ Now input value : %s ]",input_pw);
			break;
		case '*':
			if(strcmp(init_pw,input_pw)==0)
			{
				_D("[Success] Open");
				resource_write_led(4,1);
			}
			else if(strcmp(init_pw,input_pw)!=0)
			{
				_E("[Error] please retry");
				error_count++;
				if(error_count>=3)
				{
					_E("[Fail] Exceed input count");
					resource_write_buzzer(24,1);
				}
				else
				{
					goto clear;
				}
			}
			break;
		default:
			if(state==0)
			{
				_D("Now input [%c]key", getch);
				init_pw[init_index++]=getch;
			}
			else if(state==1)
			{
				_D("Now input [%c]key", getch);
				input_pw[input_index++]=getch;
			}
			break;
		}
	}

#endif
	/* This is example, get value from sensors first */
//	if (connectivity_notify_int(ad->resource_info, "Motion", value) == -1)
//		_E("Cannot notify message");

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
	ad->getter_timer = ecore_timer_add(1.5f, control_sensors_cb, ad);
	/*if (!ad->getter_timer) {
		_E("Failed to add infrared motion getter timer");
		return false;
	}*/

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
