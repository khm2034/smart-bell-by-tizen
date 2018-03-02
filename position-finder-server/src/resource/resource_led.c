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

#include "log.h"
#include "resource_internal.h"

void resource_close_led(int pin_num)
{
	if (!resource_get_info(pin_num)->opened) return;

	_I("LED is finishing...");
	peripheral_gpio_close(resource_get_info(pin_num)->sensor_h);
	resource_get_info(pin_num)->opened = 0;
}

int resource_write_led(int pin_num, int write_value)
{
	int ret = PERIPHERAL_ERROR_NONE;

	if (!resource_get_info(pin_num)->opened) {
		ret = peripheral_gpio_open(pin_num, &resource_get_info(pin_num)->sensor_h);
		retv_if(!resource_get_info(pin_num)->sensor_h, -1);

		ret = peripheral_gpio_set_direction(resource_get_info(pin_num)->sensor_h, PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
		retv_if(ret != 0, -1);

		resource_get_info(pin_num)->opened = 1;
		resource_get_info(pin_num)->close = resource_close_led;
	}

	ret = peripheral_gpio_write(resource_get_info(pin_num)->sensor_h, write_value);
	retv_if(ret < 0, -1);

	//_I("LED Value : %s", write_value ? "ON":"OFF");

	return 0;
}
