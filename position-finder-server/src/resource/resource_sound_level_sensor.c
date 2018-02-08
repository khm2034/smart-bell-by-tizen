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

#include <stdlib.h>
#include <unistd.h>
#include <peripheral_io.h>
#include <sys/time.h>

#include "log.h"
#include "resource/resource_adc_mcp3008.h"

static bool initialized = false;

void resource_close_sound_level_sensor(void)
{
	resource_adc_mcp3008_fini();
	initialized = false;
}

int resource_read_sound_level_sensor(int ch_num, unsigned int *out_value)
{
	unsigned int read_value = 0;
	int ret = 0;

	if (!initialized) {
		ret = resource_adc_mcp3008_init();
		retv_if(ret != 0, -1);
		initialized = true;
	}
	ret = resource_read_adc_mcp3008(ch_num, &read_value);
	retv_if(ret != 0, -1);

	*out_value = read_value;

	return 0;
}

