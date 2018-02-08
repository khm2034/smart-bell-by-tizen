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

#ifndef __POSITION_FINDER_RESOURCE_SOUND_LEVEL_SENSOR_H__
#define __POSITION_FINDER_RESOURCE_SOUND_LEVEL_SENSOR_H__

 /**
  * @brief Reads the value from sound level sensor through AD converter(MCP3008).
  * @remarks We assume that only one AD converter is connected with device.
  * @param[in] ch_num The number of channel connected to the sound level sensor with AD converter
  * @param[out] out_value The value of a sound level
  * @return 0 on success, otherwise a negative error value
  *
  */
extern int resource_read_sound_level_sensor(int ch_num, unsigned int *out_value);

#endif /* __POSITION_FINDER_RESOURCE_SOUND_LEVEL_SENSOR_H__ */

