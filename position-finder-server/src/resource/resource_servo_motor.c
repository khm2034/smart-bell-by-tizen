/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
 *
 * Contact: Jeonghoon Park <jh1979.park@samsung.com>
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

#include "log.h"
#include "resource/resource_PCA9685.h"
#include "resource/resource_servo_motor.h"
#define SERVO_MOTOR_MAX PCA9685_CH_MAX

static int servo_motor_index[SERVO_MOTOR_MAX + 1] = {0, };

static int resource_servo_motor_init(unsigned int ch)
{
	int ret = 0;
	_D("test3");
	ret = resource_pca9685_init(ch);
	_D("test4");
	if (ret) {
		_E("failed to init PCA9685 with ch[%u]", ch);
		return -1;
	}
	servo_motor_index[ch] = 1;

	return 0;
}

void resource_close_servo_motor(unsigned int ch)
{
	if (servo_motor_index[ch] == 1) {
		resource_pca9685_fini(ch);
		servo_motor_index[ch] = 0;
	}

	return;
}

void resource_close_servo_motor_all(void)
{
	unsigned int i;

	for (i = 0 ; i <= SERVO_MOTOR_MAX; i++)
		resource_close_servo_motor(i);

	return;
}

int resource_set_servo_motor_value(unsigned int motor_id, int value)
{
	int ret = 0;

	if (motor_id > SERVO_MOTOR_MAX)
		return -1;

	if (servo_motor_index[motor_id] == 0) {
		ret = resource_servo_motor_init(motor_id);
		if (ret)
			return -1;
	}

	_D("test1");
	ret = resource_pca9685_set_value_to_channel(motor_id, 0, value);
	_D("test2");
	return ret;
}
