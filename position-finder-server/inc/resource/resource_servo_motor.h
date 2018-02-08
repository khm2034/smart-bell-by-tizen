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

#ifndef __RESOURCE_SERVO_MOTOR_H__
#define __RESOURCE_SERVO_MOTOR_H__

/**
 * This module is sample codes to handling Servo motors in Tizen platform.
 * HW is configured with PCA9685(PWM controller).
 */

/**
 * @param[in] id The motor id
 * @param[in] value The value to control servo motor
 *
 * @return 0 on success, otherwise a negative error value
 * @remarks Must adjust servo motor with some value before use to fit your system.
 */
int resource_set_servo_motor_value(unsigned int motor_id, int value);

#endif /* __RESOURCE_SERVO_MOTOR_H__ */
