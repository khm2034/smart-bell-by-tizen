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

#include <peripheral_io.h>

#include "log.h"
#include "resource.h"

static resource_s resource_info[PIN_MAX] = { {0, NULL, NULL}, };

resource_s *resource_get_info(int pin_num)
{
	return &resource_info[pin_num];
}

void resource_close_all(void)
{
	int i = 0;
	for (i = 0; i < PIN_MAX; i++) {
		if (!resource_info[i].opened) continue;
		_I("GPIO[%d] is closing...", i);

		if (resource_info[i].close)
			resource_info[i].close(i);
	}
	resource_close_illuminance_sensor();
	resource_close_sound_level_sensor();
}
