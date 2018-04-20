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
	motor_state_e motor_state;
	peripheral_gpio_h pin1_h;
	peripheral_gpio_h pin2_h;
	peripheral_gpio_h pin3_h;
	peripheral_gpio_h pin4_h;
	unsigned long long thread_id;
	int thread_state;
} stepper_motor_driver_s;

static stepper_motor_driver_s g_md_step_h[ULN2003_MOTOR_ID_MAX] = {
	{0, 0, 0, 0, MOTOR_STATE_NONE, NULL, NULL, NULL, NULL, 0L, 0},
	{0, 0, 0, 0, MOTOR_STATE_NONE, NULL, NULL, NULL, NULL, 0L, 0}
};

static int motor_id[2]= {0, 1};
static int step = 0;
static int cur_delay = 0;
static int __motor_brake_n_stop_by_id(ULN2003_step_motor_id_e id)
{
	int ret = PERIPHERAL_ERROR_NONE;
	if(g_md_step_h[id].thread_id != 0L){
		g_md_step_h[id].thread_state = 0;
		pthread_join(g_md_step_h[id].thread_id, (void *)&ret);
		if (ret) {
			_E("failed to stop motor[%d]", id);
			return -1;
		}
		g_md_step_h[id].thread_id = 0;
	}
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
	ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[0] Motor[%d] pin 1", 0, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[0] Motor[%d] pin 2", 0, id);
		return -1;
	}
	ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[0] Motor[%d] pin 3", 0, id);
		return -1;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("Failed to set value[0] Motor[%d] pin 4", 0, id);
		return -1;
	}
	g_md_step_h[id].motor_state = MOTOR_STATE_STOP;

	return 0;
}

static int __set_default_configuration_by_id(ULN2003_step_motor_id_e id)
{
	unsigned int pin_1, pin_2, pin_3, pin_4;
	switch (id) {
	case ULN2003_MOTOR_ID_1:
		pin_1 = DEFAULT_STEP_MOTOR1_PIN1;
		pin_2 = DEFAULT_STEP_MOTOR1_PIN2;
		pin_3 = DEFAULT_STEP_MOTOR1_PIN3;
		pin_4 = DEFAULT_STEP_MOTOR1_PIN4;
	break;
	case ULN2003_MOTOR_ID_2:
		pin_1 = DEFAULT_STEP_MOTOR2_PIN1;
		pin_2 = DEFAULT_STEP_MOTOR2_PIN2;
		pin_3 = DEFAULT_STEP_MOTOR2_PIN3;
		pin_4 = DEFAULT_STEP_MOTOR2_PIN4;
	break;
	case ULN2003_MOTOR_ID_MAX:
	default:
		_E("Unkwon ID[%d]", id);
		return -1;
	break;
	}

	g_md_step_h[id].pin_1 = pin_1;
	g_md_step_h[id].pin_2 = pin_2;
	g_md_step_h[id].pin_3 = pin_3;
	g_md_step_h[id].pin_4 = pin_4;

	g_md_step_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

static int __fini_motor_by_id(ULN2003_step_motor_id_e id)
{
	step = 0;
	retv_if(id == ULN2003_MOTOR_ID_MAX, -1);

	if (g_md_step_h[id].motor_state <= MOTOR_STATE_CONFIGURED)
		return 0;

	if (g_md_step_h[id].motor_state > MOTOR_STATE_STOP)
		__motor_brake_n_stop_by_id(id);

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

	g_md_step_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}


static int __init_motor_by_id(ULN2003_step_motor_id_e id)
{
	int ret = 0;

	retv_if(id == ULN2003_MOTOR_ID_MAX, -1);

	if (g_md_step_h[id].motor_state == MOTOR_STATE_NONE)
		__set_default_configuration_by_id(id);

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

	return -1;
}

void resource_close_motor_driver_ULN2003(ULN2003_step_motor_id_e id)
{
	__fini_motor_by_id(id);
	return;
}

void resource_close_motor_driver_ULN2003_all(void)
{
	int i;
	for (i = ULN2003_MOTOR_ID_1; i < ULN2003_MOTOR_ID_MAX; i++)
		__fini_motor_by_id(i);

	return;
}

int resource_set_motor_driver_ULN2003_configuration(ULN2003_step_motor_id_e id,
		unsigned int pin1, unsigned int pin2, unsigned int pin3, unsigned int pin4)
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
	g_md_step_h[id].motor_state = MOTOR_STATE_CONFIGURED;

	return 0;
}

int _resource_motor_ULN2003_looping(void* data1)
{
	_D("id : %d", *((int*)data1));
	int id = *((int*)data1);
	_D("id : %d", id);
	int ret = 0;
	ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[0] Motor[%d] pin 1[%d]", id, g_md_step_h[id].pin_1);
		goto ERROR;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[0] Motor[%d] pin 2[%d]", id, g_md_step_h[id].pin_2);
		goto ERROR;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[0] Motor[%d] pin 3[%d]", id, g_md_step_h[id].pin_3);
		goto ERROR;
	}

	ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, 0);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to set value[0] Motor[%d] pin 4[%d]", id, g_md_step_h[id].pin_4);
		goto ERROR;
	}
	while(1){
		if(g_md_step_h[id].thread_state == 0)
			break;
		switch(step){
		case ULN2003_MOTOR_STEP_1:
			ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 1);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[1] Motor[%d] pin 1", id);
				goto ERROR;
			}
			ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, 1);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[1] Motor[%d] pin 2", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_2;
			else
				step = ULN2003_MOTOR_STEP_8;
			break;
		case ULN2003_MOTOR_STEP_2:
			ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 0);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[0] Motor[%d] pin 1", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_3;
			else
				step = ULN2003_MOTOR_STEP_1;
			break;
		case ULN2003_MOTOR_STEP_3:
			ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, 1);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[1] Motor[%d] pin 3", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_4;
			else
				step = ULN2003_MOTOR_STEP_2;
			break;
		case ULN2003_MOTOR_STEP_4:
			ret = peripheral_gpio_write(g_md_step_h[id].pin2_h, 0);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[0] Motor[%d] pin 2", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_5;
			else
				step = ULN2003_MOTOR_STEP_3;
			break;
		case ULN2003_MOTOR_STEP_5:
			ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, 1);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[1] Motor[%d] pin 4", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_6;
			else
				step = ULN2003_MOTOR_STEP_4;
			break;
		case ULN2003_MOTOR_STEP_6:
			ret = peripheral_gpio_write(g_md_step_h[id].pin3_h, 0);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[0] Motor[%d] pin 3", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_7;
			else
				step = ULN2003_MOTOR_STEP_5;
			break;
		case ULN2003_MOTOR_STEP_7:
			ret = peripheral_gpio_write(g_md_step_h[id].pin1_h, 1);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[1] Motor[%d] pin 1", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_8;
			else
				step = ULN2003_MOTOR_STEP_6;
			break;
		case ULN2003_MOTOR_STEP_8:
			ret = peripheral_gpio_write(g_md_step_h[id].pin4_h, 0);
			if (ret != PERIPHERAL_ERROR_NONE) {
				_E("failed to set value[0] Motor[%d] pin 4", id);
				goto ERROR;
			}
			if(g_md_step_h[id].motor_state == MOTOR_STATE_FORWARD)
				step = ULN2003_MOTOR_STEP_1;
			else
				step = ULN2003_MOTOR_STEP_7;
			break;
		}

		usleep(cur_delay);
	}
	return ret;

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

	return -1;
}

int resource_set_motor_driver_ULN2003_speed(ULN2003_step_motor_id_e id, int delay, int dir)
{
	int ret = 0;
	int e_state = MOTOR_STATE_NONE;
	int thr_id;
	pthread_t thread;
	if (g_md_step_h[id].motor_state <= MOTOR_STATE_CONFIGURED) {
		ret = __init_motor_by_id(id);
		if (ret) {
			_E("failed to __init_motor_by_id()");
			return -1;
		}
	}

	if (dir == -1) {
		ret = __motor_brake_n_stop_by_id(id);
		if (ret) {
			_E("failed to stop motor[%d]", id);
			return -1;
		}
		return 0; /* done */
	}

	if(dir)
		e_state = MOTOR_STATE_FORWARD;
	else
		e_state = MOTOR_STATE_BACKWARD;

	if(g_md_step_h[id].motor_state == e_state)
		goto SET_SPEED;
	else{
		ret = __motor_brake_n_stop_by_id(id);
		if (ret) {
			_E("failed to stop motor[%d]", id);
			return -1;
		}
	}
	if (e_state == MOTOR_STATE_FORWARD)
		step = ULN2003_MOTOR_STEP_8;
	else
		step = ULN2003_MOTOR_STEP_1; /* will be set backward */

	g_md_step_h[id].motor_state = e_state;
	g_md_step_h[id].thread_state = 1;
	_D("id1 : %d", id);;
	thr_id = pthread_create(&thread, NULL, _resource_motor_ULN2003_looping, (void*)&motor_id[id]);
	_D("id2 : %d", id);
	if(thr_id <0)
	{
		_E("Failed to create thread");
		return -1;
	}
	g_md_step_h[id].thread_id = thread;

	return 0;

SET_SPEED:
	cur_delay = delay;
	return 0;
}
