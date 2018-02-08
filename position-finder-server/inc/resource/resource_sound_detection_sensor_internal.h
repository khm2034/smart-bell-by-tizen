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

#ifndef __POSITION_FINDER_RESOURCE_SOUND_DETECTION_SENSOR_INTERNAL_H__
#define __POSITION_FINDER_RESOURCE_SOUND_DETECTION_SENSOR_INTERNAL_H__

/**
 * @brief Releases the gpio handle and changes the gpio pin state to the close(0).
 * @param[in] pin_num The number of the gpio pin connected to the sound detection sensor
 */
extern void resource_close_sound_detection_sensor(int pin_num);

#endif /* __POSITION_FINDER_RESOURCE_SOUND_DETECTION_SENSOR_INTERNAL_H__ */
