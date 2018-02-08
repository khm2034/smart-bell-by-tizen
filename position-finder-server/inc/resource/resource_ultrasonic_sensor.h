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

#ifndef __POSITION_FINDER_RESOURCE_ULTRASONIC_SENSOR_H__
#define __POSITION_FINDER_RESOURCE_ULTRASONIC_SENSOR_H__

/**
 * @brief Reads the value of gpio connected ultrasonic sensor(HC-SR04).
 * @param[in] trig_pin_num The number of the gpio pin connected to the trig of the ultrasonic sensor
 * @param[in] echo_pin_num The number of the gpio pin connected to the echo of the ultrasonic sensor
 * @param[in] cb A callback function to be invoked when the gpio interrupt is triggered
 * @param[in] data The data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value
 * @see If the gpio pin is not open, creates gpio handle before reading the value of gpio.
 */
extern int resource_read_ultrasonic_sensor(int trig_pin_num, int echo_pin_num, resource_read_cb cb, void *data);

#endif /* __POSITION_FINDER_RESOURCE_ULTRASONIC_SENSOR_H__ */
