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

#include <stdlib.h>
#include <unistd.h>
#include <peripheral_io.h>
#include <sys/time.h>
#include <gio/gio.h>

#include "log.h"
#include "resource_internal.h"

static resource_read_s *resource_read_info = NULL;
static unsigned long long triggered_time = 0;

void resource_close_ultrasonic_sensor_trig(int trig_pin_num)
{
	if (!resource_get_info(trig_pin_num)->opened) return;

	_I("Ultrasonic sensor's trig is finishing...");

	peripheral_gpio_close(resource_get_info(trig_pin_num)->sensor_h);
	resource_get_info(trig_pin_num)->opened = 0;
}

void resource_close_ultrasonic_sensor_echo(int echo_pin_num)
{
	if (!resource_get_info(echo_pin_num)->opened) return;

	_I("Ultrasonic sensor's echo is finishing...");

	peripheral_gpio_close(resource_get_info(echo_pin_num)->sensor_h);
	resource_get_info(echo_pin_num)->opened = 0;
	free(resource_read_info);
	resource_read_info = NULL;
}

static unsigned long long _get_timestamp(void)
{
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return ((unsigned long long)(t.tv_sec)*1000000000LL + t.tv_nsec) / 1000;
}

static void _resource_read_ultrasonic_sensor_cb(peripheral_gpio_h gpio, peripheral_error_e error, void *user_data)
{
	float dist = 0;
	uint32_t value;
	unsigned long long returned_time = 0;
	resource_read_s *resource_read_info = user_data;

	ret_if(!resource_read_info);
	ret_if(!resource_read_info->cb);

	_D("value1 : %d", value);
	peripheral_gpio_read(gpio, &value);
	_D("value2 : %d", value);
	if (value == 1) {
		triggered_time = _get_timestamp();
	} else if (value == 0) {
		returned_time = _get_timestamp();
	}

	_D("trig time : %llu, value : %d, return_time : %llu", triggered_time, value, returned_time);
	if (triggered_time > 0 && value == 0) {
		_D("into if");
		dist = returned_time - triggered_time;
		_D("dist : %f", dist);
		if (dist < 150 || dist > 25000) {
			dist = -1;
		} else {
			dist = (dist * 34300) / 2000000;
		}
		_D("set Dist : %lf", dist);
		resource_read_info->cb((double)dist, resource_read_info->data);
	}
}

int resource_read_ultrasonic_sensor(int trig_pin_num, int echo_pin_num, resource_read_cb cb, void *data)
{
	int ret = 0;

	triggered_time = 0;

	if (resource_read_info == NULL) {
		resource_read_info = calloc(1, sizeof(resource_read_s));
		retv_if(!resource_read_info, -1);
	} else {
		peripheral_gpio_unset_interrupted_cb(resource_get_info(resource_read_info->pin_num)->sensor_h);
	}
	resource_read_info->cb = cb;
	resource_read_info->data = data;
	resource_read_info->pin_num = echo_pin_num;

	if (!resource_get_info(trig_pin_num)->opened) {
		_I("Ultrasonic sensor's trig is initializing...");

		ret = peripheral_gpio_open(trig_pin_num, &resource_get_info(trig_pin_num)->sensor_h);
		retv_if(!resource_get_info(trig_pin_num)->sensor_h, -1);

		ret = peripheral_gpio_set_direction(resource_get_info(trig_pin_num)->sensor_h, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
		retv_if(ret != 0, -1);

		resource_get_info(trig_pin_num)->opened = 1;
		resource_get_info(trig_pin_num)->close = resource_close_ultrasonic_sensor_trig;
	}

	if (!resource_get_info(echo_pin_num)->opened) {
		_I("Ultrasonic sensor's echo is initializing...");

		ret = peripheral_gpio_open(echo_pin_num, &resource_get_info(echo_pin_num)->sensor_h);
		retv_if(!resource_get_info(echo_pin_num)->sensor_h, -1);

		ret = peripheral_gpio_set_direction(resource_get_info(echo_pin_num)->sensor_h, PERIPHERAL_GPIO_DIRECTION_IN);
		retv_if(ret != 0, -1);

		ret = peripheral_gpio_set_edge_mode(resource_get_info(echo_pin_num)->sensor_h, PERIPHERAL_GPIO_EDGE_BOTH);
		retv_if(ret != 0, -1);

		resource_get_info(echo_pin_num)->opened = 1;
		resource_get_info(echo_pin_num)->close = resource_close_ultrasonic_sensor_echo;
	}

	if (resource_get_info(echo_pin_num)->sensor_h) {
		ret = peripheral_gpio_set_interrupted_cb(resource_get_info(echo_pin_num)->sensor_h, _resource_read_ultrasonic_sensor_cb, resource_read_info);
		retv_if(ret != 0, -1);
	}

	ret = peripheral_gpio_write(resource_get_info(trig_pin_num)->sensor_h, 0);
	retv_if(ret < 0, -1);

	usleep(20000);

	ret = peripheral_gpio_write(resource_get_info(trig_pin_num)->sensor_h, 1);
	retv_if(ret < 0, -1);

	usleep(20000);

	ret = peripheral_gpio_write(resource_get_info(trig_pin_num)->sensor_h, 0);
	retv_if(ret < 0, -1);


	return 0;
}
