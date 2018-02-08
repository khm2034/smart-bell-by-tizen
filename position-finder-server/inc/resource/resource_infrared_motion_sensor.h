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

#ifndef __POSITION_FINDER_RESOURCE_INFRARED_MOTION_SENSOR_H__
#define __POSITION_FINDER_RESOURCE_INFRARED_MOTION_SENSOR_H__

/**
 * @brief Reads the value of gpio connected infrared motion sensor(HC-SR501).
 * @param[in] pin_num The number of the gpio pin connected to the infrared motion sensor
 * @param[out] out_value The value of the gpio (zero or non-zero)
 * @return 0 on success, otherwise a negative error value
 * @see If the gpio pin is not open, creates gpio handle before reading the value of gpio.
 */
extern int resource_read_infrared_motion_sensor(int pin_num, uint32_t *out_value);

#endif /* __POSITION_FINDER_RESOURCE_INFRARED_MOTION_SENSOR_H__ */
