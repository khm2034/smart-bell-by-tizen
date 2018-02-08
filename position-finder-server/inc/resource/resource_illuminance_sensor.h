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

#ifndef __POSITION_FINDER_RESOURCE_ILLUMINANCE_SENSOR_H__
#define __POSITION_FINDER_RESOURCE_ILLUMINANCE_SENSOR_H__

/**
 * @brief Reads the value of i2c bus connected illuminance sensor.
 * @param[in] i2c_bus The i2c bus number that the slave device is connected
 * @param[out] out_value The value read by the illuminance sensor
 * @return 0 on success, otherwise a negative error value
 * @see If the i2c bus is not open, creates i2c handle before reading data from the i2c slave device.
 */
extern int resource_read_illuminance_sensor(int i2c_bus, uint32_t *out_value);

#endif /* __POSITION_FINDER_RESOURCE_ILLUMINANCE_SENSOR_H__ */

