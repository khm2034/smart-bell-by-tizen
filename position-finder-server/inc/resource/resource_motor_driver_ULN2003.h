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

#ifndef __RESOURCE_MOTOR_DRIVER_ULN2003_H__
#define __RESOURCE_MOTOR_DRIVER_ULN2003_H__

/* Default GPIO pins of raspberry pi 3 connected with IN pins of L298N for stepper*/
#define DEFAULT_STEP_MOTOR1_PIN1 26
#define DEFAULT_STEP_MOTOR1_PIN2 20
#define DEFAULT_STEP_MOTOR1_PIN3 19
#define DEFAULT_STEP_MOTOR1_PIN4 16

#define DEFAULT_STEP_MOTOR2_PIN1 6
#define DEFAULT_STEP_MOTOR2_PIN2 12
#define DEFAULT_STEP_MOTOR2_PIN3 22
#define DEFAULT_STEP_MOTOR2_PIN4 23

#define ENABLE_A_PIN 22
#define ENABLE_B_PIN 27


/**
 * @brief Enumeration for motor id.
 */

typedef enum {
	ULN2003_STEP_MOTOR_ID_1,
	ULN2003_STEP_MOTOR_ID_2,
	ULN2003_STEP_MOTOR_ID_MAX
} ULN2003_step_motor_id_e;

typedef enum {
	ULN2003_MOTOR_STEP_1 = 1,
	ULN2003_MOTOR_STEP_2,
	ULN2003_MOTOR_STEP_3,
	ULN2003_MOTOR_STEP_4,
	ULN2003_MOTOR_STEP_5,
	ULN2003_MOTOR_STEP_6,
	ULN2003_MOTOR_STEP_7,
	ULN2003_MOTOR_STEP_8,
}ULN2003_motor_step;

int resource_set_stepper_motor_driver_ULN2003_speed(ULN2003_step_motor_id_e id, int delay);

int resource_set_stepper_motor_driver_ULN2003_configuration(ULN2003_step_motor_id_e id,
		unsigned int pin1, unsigned int pin2, unsigned en_ch1,
		unsigned int pin3, unsigned int pin4, unsigned en_ch2);

#endif /* __RESOURCE_MOTOR_DRIVER_L298N_H__ */
