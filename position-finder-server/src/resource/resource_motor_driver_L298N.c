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
#include "resource/resource_PCA9685.h"
#include "resource/resource_motor_driver_L298N.h"

typedef enum {
	MOTOR_STATE_NONE,
	MOTOR_STATE_CONFIGURED,
	MOTOR_STATE_STOP,
	MOTOR_STATE_FORWARD,
	MOTOR_STATE_BACKWARD,
} motor_state_e;

typedef struct __motor_driver_s {
	unsigned int pin_1;
	unsigned int pin_2;
	unsigned int en_ch;
	motor_state_e motor_state;
	peripheral_gpio_h pin1_h;
	peripheral_gpio_h pin2_h;
} motor_driver_s;
static motor_driver_s g_md_dc_h[MOTOR_ID_MAX] = {
	{0, 0, 0, MOTOR_STATE_NONE, NULL, NULL},
};

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

static stepper_motor_driver_s g_md_step_h[STEP_MOTOR_ID_MAX] = {
	{0, 0, 0, 0, 0, 0, MOTOR_STATE_NONE, NULL, NULL, NULL, NULL, NULL, NULL},
};

static int step = 0;
static int thread_state = 0;
static int cur_delay = 0;
static int thread_ret = 0;
/* see Principle section in http://wiki.sunfounder.cc/index.php?title=Motor_Driver_Module-L298N */

static int __dc_motor_brake_n_stop_by_id(motor_id_e id)
{
	int ret = PERIPHERAL_ERROR_NONE;
	int motor1_v = 0;
	int motor2_v = 0;

	if (g_md_dc_h[id].motor_state <= MOTOR_STATE_CONFIGURED) {
		_E("motor[%d] are not initialized - state(%d)",
			id, g_md_dc_h[id].motor_state);
		return -1;
	}

	if (g_md_dc_h[id].motor_state == MOTOR_STATE_STOP) {
		_D("motor[%d] is already stopped", id);
		return 0;
	}

	if (g_md_dc_h[id].motor_state == MOTOR_STATE_FORWARD) {
		motor1_v = 0;
		motor2_v = 0;
	} else if (g_md_dc_h[id].motor_state == MOTOR_STATE_BACKWARD) {
		motor1_v = 1;
		motor2_v = 1;
	}

	/* Brake DC motor */
	ret = peripheral_gpio_write(g_md_dc_h[id].pin1_h, motor1_v);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin 1", motor1_v, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_dc_h[id].pin2_h, motor2_v);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[%d] Motor[%d] pin 2", motor2_v, id);
		return -1;
	}

	/* set stop DC motor */
	// need to stop motor or not?, it may stop motor to free running
	resource_pca9685_set_value_to_channel(g_md_dc_h[id].en_ch, 0, 0);

	g_md_dc_h[id].motor_state = MOTOR_STATE_STOP;

	return 0;
}

static int __stepper_motor_brake_n_stop_by_id(step_motor_id_e id)
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
static int __dc_set_default_configuration_by_id(motor_id_e id)
{
	unsigned int pin_1, pin_2, en_ch;

	switch (id) {
	case MOTOR_ID_1:
		pin_1 = DEFAULT_MOTOR1_PIN1;
		pin_2 = DEFAULT_MOTOR1_PIN2;
		en_ch = DEFAULT_MOTOR1_EN_CH;
	break;
	case MOTOR_ID_2:
		pin_1 = DEFAULT_MOTOR2_PIN1;
		pin_2 = DEFAULT_MOTOR2_PIN2;
		en_ch = DEFAULT_MOTOR2_EN_CH;
	break;
	case MOTOR_ID_3:
		pin_1 = DEFAULT_MOTOR3_PIN1;
		pin_2 = DEFAULT_MOTOR3_PIN2;
		en_ch = DEFAULT_MOTOR3_EN_CH;
	break;
	case MOTOR_ID_4:
		pin_1 = DEFAULT_MOTOR4_PIN1;
		pin_2 = DEFAULT_MOTOR4_PIN2;
		en_ch = DEFAULT_MOTOR4_EN_CH;
	break;
	case MOTOR_ID_MAX:
	default:
		_E("Unkwon ID[%d]", id);
		return -1;
	break;
	}

	g_md_dc_h[id].pin_1 = pin_1;
	g_md_dc_h[id].pin_2 = pin_2;
	g_md_dc_h[id].en_ch = en_ch;
	g_md_dc_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

static int __stepper_set_default_configuration_by_id(step_motor_id_e id)
{
	unsigned int pin_1, pin_2, pin_3, pin_4;
	unsigned int pin_ena, pin_enb;
	pin_ena = ENABLE_A_PIN;
	pin_enb = ENABLE_B_PIN;
	switch (id) {
	case STEP_MOTOR_ID_1:
		pin_1 = DEFAULT_STEP_MOTOR1_PIN1;
		pin_2 = DEFAULT_STEP_MOTOR1_PIN2;
		pin_3 = DEFAULT_STEP_MOTOR1_PIN3;
		pin_4 = DEFAULT_STEP_MOTOR1_PIN4;
	break;
	case STEP_MOTOR_ID_2:
		pin_1 = DEFAULT_STEP_MOTOR2_PIN1;
		pin_2 = DEFAULT_STEP_MOTOR2_PIN2;
		pin_3 = DEFAULT_STEP_MOTOR2_PIN3;
		pin_4 = DEFAULT_STEP_MOTOR2_PIN4;
		pin_ena = ENABLE_A_PIN;
		pin_enb = ENABLE_B_PIN;
	break;
	case STEP_MOTOR_ID_MAX:
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

static int __dc_fini_motor_by_id(motor_id_e id)
{
	retv_if(id == MOTOR_ID_MAX, -1);

	if (g_md_dc_h[id].motor_state <= MOTOR_STATE_CONFIGURED)
		return 0;

	if (g_md_dc_h[id].motor_state > MOTOR_STATE_STOP)
		__dc_motor_brake_n_stop_by_id(id);

	resource_pca9685_fini(g_md_dc_h[id].en_ch);

	if (g_md_dc_h[id].pin1_h) {
		peripheral_gpio_close(g_md_dc_h[id].pin1_h);
		g_md_dc_h[id].pin1_h = NULL;
	}

	if (g_md_dc_h[id].pin2_h) {
		peripheral_gpio_close(g_md_dc_h[id].pin2_h);
		g_md_dc_h[id].pin2_h = NULL;
	}

	g_md_dc_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

static int __stepper_fini_motor_by_id(step_motor_id_e id)
{
	step = 0;
	thread_state = 0;
	retv_if(id == STEP_MOTOR_ID_MAX, -1);

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

static int __dc_init_motor_by_id(motor_id_e id)
{
	int ret = 0;

	retv_if(id == MOTOR_ID_MAX, -1);

	if (g_md_dc_h[id].motor_state == MOTOR_STATE_NONE)
		__dc_set_default_configuration_by_id(id);

	ret = resource_pca9685_init(g_md_dc_h[id].en_ch);
	if (ret) {
		_E("failed to init PCA9685");
		return -1;
	}

	/* open pins for Motor */
	ret = peripheral_gpio_open(g_md_dc_h[id].pin_1, &g_md_dc_h[id].pin1_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_dc_h[id].pin1_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin1[%u]", id, g_md_dc_h[id].pin_1);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_md_dc_h[id].pin_2, &g_md_dc_h[id].pin2_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_md_dc_h[id].pin2_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open Motor[%d] gpio pin2[%u]", id, g_md_dc_h[id].pin_2);
		goto ERROR;
	}

	g_md_dc_h[id].motor_state = MOTOR_STATE_STOP;

	return 0;

ERROR:
	resource_pca9685_fini(g_md_dc_h[id].en_ch);

	if (g_md_dc_h[id].pin1_h) {
		peripheral_gpio_close(g_md_dc_h[id].pin1_h);
		g_md_dc_h[id].pin1_h = NULL;
	}

	if (g_md_dc_h[id].pin2_h) {
		peripheral_gpio_close(g_md_dc_h[id].pin2_h);
		g_md_dc_h[id].pin2_h = NULL;
	}

	return -1;
}

static int __stepper_init_motor_by_id(step_motor_id_e id)
{
	int ret = 0;

	retv_if(id == STEP_MOTOR_ID_MAX, -1);

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

void resource_close_dc_motor_driver_L298N(motor_id_e id)
{
	__dc_fini_motor_by_id(id);
	return;
}

void resource_close_stepper_motor_driver_L298N(step_motor_id_e id)
{
	__stepper_fini_motor_by_id(id);
	return;
}

void resource_close_dc_motor_driver_L298N_all(void)
{
	int i;
	for (i = MOTOR_ID_1; i < MOTOR_ID_MAX; i++)
		__dc_fini_motor_by_id(i);

	return;
}

void resource_close_stepper_motor_driver_L298N_all(void)
{
	int i;
	for (i = STEP_MOTOR_ID_1; i < STEP_MOTOR_ID_MAX; i++)
		__stepper_fini_motor_by_id(i);

	return;
}

int resource_set_dc_motor_driver_L298N_configuration(motor_id_e id,
	unsigned int pin1, unsigned int pin2, unsigned en_ch)
{

	if (g_md_dc_h[id].motor_state > MOTOR_STATE_CONFIGURED) {
		_E("cannot set configuration motor[%d] in this state[%d]",
			id, g_md_dc_h[id].motor_state);
		return -1;
	}

	g_md_dc_h[id].pin_1 = pin1;
	g_md_dc_h[id].pin_2 = pin2;
	g_md_dc_h[id].en_ch = en_ch;
	g_md_dc_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

int resource_set_stepper_motor_driver_L298N_configuration(step_motor_id_e id,
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

int resource_set_dc_motor_driver_L298N_speed(motor_id_e id, int speed)
{
	int ret = 0;
	const int value_max = 4095;
	int value = 0;
	int e_state = MOTOR_STATE_NONE;
	int motor_v_1 = 0;
	int motor_v_2 = 0;

	if (g_md_dc_h[id].motor_state <= MOTOR_STATE_CONFIGURED) {
		ret = __dc_init_motor_by_id(id);
		if (ret) {
			_E("failed to __init_motor_by_id()");
			return -1;
		}
	}

	value = abs(speed);

	if (value > value_max) {
		value = value_max;
		_D("max speed is %d", value_max);
	}
	_D("set speed %d", value);

	if (speed == 0) {
		/* brake and stop */
		ret = __dc_motor_brake_n_stop_by_id(id);
		if (ret) {
			_E("failed to stop motor[%d]", id);
			return -1;
		}
		return 0; /* done */
	}

	if (speed > 0)
		e_state = MOTOR_STATE_FORWARD; /* will be set forward */
	else
		e_state = MOTOR_STATE_BACKWARD; /* will be set backward */

	if (g_md_dc_h[id].motor_state == e_state)
		goto SET_SPEED;
	else {
		/* brake and stop */
		ret = __dc_motor_brake_n_stop_by_id(id);
		if (ret) {
			_E("failed to stop motor[%d]", id);
			return -1;
		}
	}

	switch (e_state) {
	case MOTOR_STATE_FORWARD:
		motor_v_1 = 1;
		motor_v_2 = 0;
		break;
	case MOTOR_STATE_BACKWARD:
		motor_v_1 = 0;
		motor_v_2 = 1;
		break;
	}
	ret = peripheral_gpio_write(g_md_dc_h[id].pin1_h, motor_v_1);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[%d] Motor[%d] pin 1", motor_v_1, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_dc_h[id].pin2_h, motor_v_2);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[%d] Motor[%d] pin 2", motor_v_2, id);
		return -1;
	}

SET_SPEED:
	ret = resource_pca9685_set_value_to_channel(g_md_dc_h[id].en_ch, 0, value);
	if (ret) {
		_E("failed to set speed - %d", speed);
		return -1;
	}

	g_md_dc_h[id].motor_state = e_state;

	return 0;
}

void* _resource_stepper_motor_looping(void* data)
{
	int motor_v_1 = 0;
	int motor_v_2 = 0;
	int motor_v_3 = 0;
	int motor_v_4 = 0;
	int id = *((int*)data);

	while(1){
		_D("looping start... thread_state : %d", thread_state);
		if(thread_state == 0)
			break;

		_D("looping... step : %d", step);
		switch(step){
		case MOTOR_STEP_1:
			motor_v_1 = 1; motor_v_2 = 1; motor_v_3 = 0; motor_v_4 = 0;
			step = MOTOR_STEP_2;
			break;
		case MOTOR_STEP_2:
			motor_v_1 = 0; motor_v_2 = 1; motor_v_3 = 0; motor_v_4 = 0;
			step = MOTOR_STEP_3;
			break;
		case MOTOR_STEP_3:
			motor_v_1 = 0; motor_v_2 = 1; motor_v_3 = 1; motor_v_4 = 0;
			step = MOTOR_STEP_4;
			break;
		case MOTOR_STEP_4:
			motor_v_1 = 0; motor_v_2 = 0; motor_v_3 = 1; motor_v_4 = 0;
			step = MOTOR_STEP_5;
			break;
		case MOTOR_STEP_5:
			motor_v_1 = 0; motor_v_2 = 0; motor_v_3 = 1; motor_v_4 = 1;
			step = MOTOR_STEP_6;
			break;
		case MOTOR_STEP_6:
			motor_v_1 = 0; motor_v_2 = 0; motor_v_3 = 0; motor_v_4 = 1;
			step = MOTOR_STEP_7;
			break;
		case MOTOR_STEP_7:
			motor_v_1 = 1; motor_v_2 = 0; motor_v_3 = 0; motor_v_4 = 1;
			step = MOTOR_STEP_8;
			break;
		case MOTOR_STEP_8:
			motor_v_1 = 1; motor_v_2 = 0; motor_v_3 = 0; motor_v_4 = 0;
			step = MOTOR_STEP_1;
			break;
		}
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

		usleep(cur_delay);
	}
	_D("looping end... thread_state : %d", thread_state);
	thread_ret = 0;
	return (void*)&thread_ret;
}

int resource_set_stepper_motor_driver_L298N_speed(step_motor_id_e id, int delay)
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
	step = MOTOR_STEP_8;

	thr_id = pthread_create(&thread, NULL, _resource_stepper_motor_looping, (void*)&id);
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
