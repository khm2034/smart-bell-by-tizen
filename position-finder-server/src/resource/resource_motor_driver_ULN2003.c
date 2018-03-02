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

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <peripheral_io.h>
#include "log.h"
#include "resource/resource_motor_driver_ULN2003.h"

typedef enum {
	MOTOR_STATE_NONE,
	MOTOR_STATE_CONFIGURED,
	MOTOR_STATE_STOP,
	MOTOR_STATE_FORWARD,
	MOTOR_STATE_BACKWARD,
} motor_state_e;

/*support stepper motor*/
typedef struct __stepper_motor_driver_s {
	unsigned int pin_1;
	unsigned int pin_2;
	unsigned int pin_3;
	unsigned int pin_4;
	unsigned int pin_ena;
	unsigned int pin_enb;
	motor_state_e motor_state;
	peripheral_gpio_h pin1_h;
	peripheral_gpio_h pin2_h;
	peripheral_gpio_h pin3_h;
	peripheral_gpio_h pin4_h;
	peripheral_gpio_h pin_ena_h;
	peripheral_gpio_h pin_enb_h;
} stepper_motor_driver_s;

static stepper_motor_driver_s g_md_step_h[ULN2003_STEP_MOTOR_ID_MAX] = {
	{0, 0, 0, 0, 0, 0, MOTOR_STATE_NONE, NULL, NULL, NULL, NULL, NULL, NULL},
};

static int step = 0;
static int thread_state = 0;
static int cur_delay = 0;
static int thread_ret = 0;
/* see Principle section in http://wiki.sunfounder.cc/index.php?title=Motor_Driver_Module-ULN2003 */

static int __stepper_motor_brake_n_stop_by_id(ULN2003_step_motor_id_e id)
{
	int ret = PERIPHERAL_ERROR_NONE;
	int motor1_v = 0;
	int motor2_v = 0;
	int motor3_v = 0;
	int motor4_v = 0;
	thread_state = 0;
	if (g_md_step_h[id].motor_state <= MOTOR_STATE_CONFIGURED) {
		_E("motor[%d] are not initialized - state(%d)",
			id, g_md_step_h[id].motor_state);
		return -1;
	}

	if (g_md_step_h[id].motor_state == MOTOR_STATE_STOP) {
		_D("motor[%d] is already stopped", id);
		return 0;
	}

	/* Brake STEPPER motor */
	ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, motor1_v);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin 1", motor1_v, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, motor2_v);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin 2", motor2_v, id);
		return -1;
	}
	ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, motor3_v);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin 3", motor3_v, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, motor4_v);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin 4", motor4_v, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin_ena_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin ena", 0, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin_enb_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin enb", 0, id);
		return -1;
	}

	g_md_step_h[id].motor_state = MOTOR_STATE_STOP;

	return 0;
}

static int __stepper_set_default_configuration_by_id(ULN2003_step_motor_id_e id)
{
	unsigned int pin_1, pin_2, pin_3, pin_4;
	unsigned int pin_ena, pin_enb;
	pin_ena = ENABLE_A_PIN;
	pin_enb = ENABLE_B_PIN;
	switch (id) {
	case ULN2003_STEP_MOTOR_ID_1:
		pin_1 = DEFAULT_STEP_MOTOR1_PIN1;
		pin_2 = DEFAULT_STEP_MOTOR1_PIN2;
		pin_3 = DEFAULT_STEP_MOTOR1_PIN3;
		pin_4 = DEFAULT_STEP_MOTOR1_PIN4;
	break;
	case ULN2003_STEP_MOTOR_ID_2:
		pin_1 = DEFAULT_STEP_MOTOR2_PIN1;
		pin_2 = DEFAULT_STEP_MOTOR2_PIN2;
		pin_3 = DEFAULT_STEP_MOTOR2_PIN3;
		pin_4 = DEFAULT_STEP_MOTOR2_PIN4;
		pin_ena = ENABLE_A_PIN;
		pin_enb = ENABLE_B_PIN;
	break;
	case ULN2003_STEP_MOTOR_ID_MAX:
	default:
		_E("Unkwon ID[%d]", id);
		return -1;
	break;
	}

	g_md_step_h[id].pin_1 = pin_1;
	g_md_step_h[id].pin_2 = pin_2;
	g_md_step_h[id].pin_3 = pin_3;
	g_md_step_h[id].pin_4 = pin_4;
	g_md_step_h[id].pin_ena = pin_ena;
	g_md_step_h[id].pin_enb = pin_enb;

	g_md_step_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

static int __stepper_fini_motor_by_id(ULN2003_step_motor_id_e id)
{
	step = 0;
	thread_state = 0;
	retv_if(id == ULN2003_STEP_MOTOR_ID_MAX, -1);

	if (g_md_step_h[id].motor_state <= MOTOR_STATE_CONFIGURED)
		return 0;

	if (g_md_step_h[id].motor_state > MOTOR_STATE_STOP)
		__stepper_motor_brake_n_stop_by_id(id);

	if (g_md_step_h[id].pin1_h) {
		peripheral_gpio_close(g_md_step_h[id].pin1_h);
		g_md_step_h[id].pin1_h = NULL;
	}

	if (g_md_step_h[id].pin2_h) {
		peripheral_gpio_close(g_md_step_h[id].pin2_h);
		g_md_step_h[id].pin2_h = NULL;
	}

	if (g_md_step_h[id].pin3_h) {
		peripheral_gpio_close(g_md_step_h[id].pin3_h);
		g_md_step_h[id].pin3_h = NULL;
	}

	if (g_md_step_h[id].pin4_h) {
		peripheral_gpio_close(g_md_step_h[id].pin4_h);
		g_md_step_h[id].pin4_h = NULL;
	}

	if (g_md_step_h[id].pin_ena_h) {
		peripheral_gpio_close(g_md_step_h[id].pin_ena_h);
		g_md_step_h[id].pin_ena_h = NULL;
	}

	if (g_md_step_h[id].pin_enb_h) {
		peripheral_gpio_close(g_md_step_h[id].pin_enb_h);
		g_md_step_h[id].pin_enb_h = NULL;
	}

	g_md_step_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}


static int __stepper_init_motor_by_id(ULN2003_step_motor_id_e id)
{
	int ret = 0;

	retv_if(id == ULN2003_STEP_MOTOR_ID_MAX, -1);

	if (g_md_step_h[id].motor_state == MOTOR_STATE_NONE)
		__stepper_set_default_configuration_by_id(id);

	/* open pins for Motor */
	ret = peripheral_gpio_open(g_md_step_h[id].pin_1, &g_md_step_h[id].pin1_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_step_h[id].pin1_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin1[%u]", id, g_md_step_h[id].pin_1);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_md_step_h[id].pin_2, &g_md_step_h[id].pin2_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_step_h[id].pin2_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin2[%u]", id, g_md_step_h[id].pin_2);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_md_step_h[id].pin_3, &g_md_step_h[id].pin3_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_step_h[id].pin3_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin3[%u]", id, g_md_step_h[id].pin_3);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_md_step_h[id].pin_4, &g_md_step_h[id].pin4_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_step_h[id].pin4_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin4[%u]", id, g_md_step_h[id].pin_4);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_md_step_h[id].pin_ena, &g_md_step_h[id].pin_ena_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_step_h[id].pin_ena_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin_ena[%u]", id, g_md_step_h[id].pin_ena);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_md_step_h[id].pin_enb, &g_md_step_h[id].pin_enb_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_step_h[id].pin_enb_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin_enb[%u]", id, g_md_step_h[id].pin_enb);
		goto ERROR;
	}


	g_md_step_h[id].motor_state = MOTOR_STATE_STOP;

	return 0;

ERROR:

	if (g_md_step_h[id].pin1_h) {
		peripheral_gpio_close(g_md_step_h[id].pin1_h);
		g_md_step_h[id].pin1_h = NULL;
	}

	if (g_md_step_h[id].pin2_h) {
		peripheral_gpio_close(g_md_step_h[id].pin2_h);
		g_md_step_h[id].pin2_h = NULL;
	}

	if (g_md_step_h[id].pin3_h) {
		peripheral_gpio_close(g_md_step_h[id].pin3_h);
		g_md_step_h[id].pin3_h = NULL;
	}

	if (g_md_step_h[id].pin4_h) {
		peripheral_gpio_close(g_md_step_h[id].pin4_h);
		g_md_step_h[id].pin4_h = NULL;
	}

	if (g_md_step_h[id].pin_ena_h) {
		peripheral_gpio_close(g_md_step_h[id].pin_ena_h);
		g_md_step_h[id].pin_ena_h = NULL;
	}

	if (g_md_step_h[id].pin_enb_h) {
		peripheral_gpio_close(g_md_step_h[id].pin_enb_h);
		g_md_step_h[id].pin_enb_h = NULL;
	}

	return -1;
}

void resource_close_stepper_motor_driver_ULN2003(ULN2003_step_motor_id_e id)
{
	__stepper_fini_motor_by_id(id);
	return;
}

void resource_close_stepper_motor_driver_ULN2003_all(void)
{
	int i;
	for (i = ULN2003_STEP_MOTOR_ID_1; i < ULN2003_STEP_MOTOR_ID_MAX; i++)
		__stepper_fini_motor_by_id(i);

	return;
}

int resource_set_stepper_motor_driver_ULN2003_configuration(ULN2003_step_motor_id_e id,
		unsigned int pin1, unsigned int pin2, unsigned pin_ena,
		unsigned int pin3, unsigned int pin4, unsigned pin_enb)
{

	if (g_md_step_h[id].motor_state > MOTOR_STATE_CONFIGURED) {
		_E("cannot set configuration motor[%d] in this state[%d]",
			id, g_md_step_h[id].motor_state);
		return -1;
	}

	g_md_step_h[id].pin_1 = pin1;
	g_md_step_h[id].pin_2 = pin2;
	g_md_step_h[id].pin_3 = pin3;
	g_md_step_h[id].pin_4 = pin4;
	g_md_step_h[id].pin_ena = pin_ena;
	g_md_step_h[id].pin_enb = pin_enb;
	g_md_step_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

void* _resource_stepper_motor_ULN2003_looping(void* data)
{
	int motor_v_1 = 0;
	int motor_v_2 = 0;
	int motor_v_3 = 0;
	int motor_v_4 = 0;
	int id = *((int*)data);

	thread_ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, motor_v_1);
	if (thread_ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[%d] Motor[%d] pin 1", motor_v_1, id);
		thread_ret = -1;
		return (void*)&thread_ret;
	}

	thread_ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, motor_v_2);
	if (thread_ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[%d] Motor[%d] pin 2", motor_v_2, id);
		thread_ret = -1;
		return (void*)&thread_ret;
	}

	thread_ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, motor_v_3);
	if (thread_ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[%d] Motor[%d] pin 3", motor_v_3, id);
		thread_ret = -1;
		return (void*)&thread_ret;
	}

	thread_ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, motor_v_4);
	if (thread_ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[%d] Motor[%d] pin 4", motor_v_4, id);
		thread_ret = -1;
		return (void*)&thread_ret;
	}
	while(1){
		//_D("looping start... thread_state : %d", thread_state);
		if(thread_state == 0)
			break;

		//_D("looping... step : %d", step);
		switch(step){
		case ULN2003_MOTOR_STEP_1:
			//motor_v_1 = 1; motor_v_2 = 1; motor_v_3 = 0; motor_v_4 = 0;
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 1);
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, 1);
			step = ULN2003_MOTOR_STEP_2;
			break;
		case ULN2003_MOTOR_STEP_2:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 0);
			//motor_v_1 = 0; motor_v_2 = 1; motor_v_3 = 0; motor_v_4 = 0;
			step = ULN2003_MOTOR_STEP_3;
			break;
		case ULN2003_MOTOR_STEP_3:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, 1);

			//motor_v_1 = 0; motor_v_2 = 1; motor_v_3 = 1; motor_v_4 = 0;
			step = ULN2003_MOTOR_STEP_4;
			break;
		case ULN2003_MOTOR_STEP_4:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, 0);
			//motor_v_1 = 0; motor_v_2 = 0; motor_v_3 = 1; motor_v_4 = 0;
			step = ULN2003_MOTOR_STEP_5;
			break;
		case ULN2003_MOTOR_STEP_5:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, 1);
			//motor_v_1 = 0; motor_v_2 = 0; motor_v_3 = 1; motor_v_4 = 1;
			step = ULN2003_MOTOR_STEP_6;
			break;
		case ULN2003_MOTOR_STEP_6:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, 0);
			//motor_v_1 = 0; motor_v_2 = 0; motor_v_3 = 0; motor_v_4 = 1;
			step = ULN2003_MOTOR_STEP_7;
			break;
		case ULN2003_MOTOR_STEP_7:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 1);
			//motor_v_1 = 1; motor_v_2 = 0; motor_v_3 = 0; motor_v_4 = 1;
			step = ULN2003_MOTOR_STEP_8;
			break;
		case ULN2003_MOTOR_STEP_8:
			thread_ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, 0);
			//motor_v_1 = 1; motor_v_2 = 0; motor_v_3 = 0; motor_v_4 = 0;
			step = ULN2003_MOTOR_STEP_1;
			break;
		}
//		thread_ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, motor_v_1);
//		if (thread_ret != PERIPHERAL_ERROR_NONE) {
//			_E("failed to set value[%d] Motor[%d] pin 1", motor_v_1, id);
//			thread_ret = -1;
//			return (void*)&thread_ret;
//		}
//
//		thread_ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, motor_v_2);
//		if (thread_ret != PERIPHERAL_ERROR_NONE) {
//			_E("failed to set value[%d] Motor[%d] pin 2", motor_v_2, id);
//			thread_ret = -1;
//			return (void*)&thread_ret;
//		}
//
//		thread_ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, motor_v_3);
//		if (thread_ret != PERIPHERAL_ERROR_NONE) {
//			_E("failed to set value[%d] Motor[%d] pin 3", motor_v_3, id);
//			thread_ret = -1;
//			return (void*)&thread_ret;
//		}
//
//		thread_ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, motor_v_4);
//		if (thread_ret != PERIPHERAL_ERROR_NONE) {
//			_E("failed to set value[%d] Motor[%d] pin 4", motor_v_4, id);
//			thread_ret = -1;
//			return (void*)&thread_ret;
//		}

		//usleep(cur_delay);
	}
	//_D("looping end... thread_state : %d", thread_state);
	thread_ret = 0;
	return (void*)&thread_ret;
}

int resource_set_stepper_motor_driver_ULN2003_speed(ULN2003_step_motor_id_e id, int delay)
{
	int ret = 0;
	int e_state = MOTOR_STATE_NONE;

	pthread_t thread;
	int thr_id;

	if (g_md_step_h[id].motor_state <= MOTOR_STATE_CONFIGURED) {
		ret = __stepper_init_motor_by_id(id);
		if (ret) {
			_E("failed to __init_motor_by_id()");
			return -1;
		}
	}

	if(cur_delay != delay)
		cur_delay = delay;

	if (delay == 0) {
		/* brake and stop */
		ret = __stepper_motor_brake_n_stop_by_id(id);
		if (ret) {
			_E("failed to stop motor[%d]", id);
			return -1;
		}
		return 0; /* done */
	}

	if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD){
		_D("thread is operating!!!");
		return 0;
	}

	if (delay > 0)
		e_state = MOTOR_STATE_FORWARD; /* will be set forward */
	else
		e_state = MOTOR_STATE_BACKWARD; /* will be set backward */

	ret = peripheral_gpio_write(g_md_step_h[id].pin_ena_h, 1);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] ena", 1, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin_enb_h, 1);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] enb", 1, id);
		return -1;
	}
	cur_delay = delay;
	g_md_step_h[id].motor_state = e_state;
	thread_state = 1;
	step = ULN2003_MOTOR_STEP_8;

	thr_id = pthread_create(&thread, NULL, _resource_stepper_motor_ULN2003_looping, (void*)&id);
	if(thr_id <0)
	{
		_E("Failed to create thread");
		return -1;
	}
	_D("Create thread");
	pthread_detach(thread);
	_D("Detach thread");

	return 0;
}
