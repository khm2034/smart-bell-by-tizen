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
#include "resource_internal.h"

#define I2C_PIN_MAX 28
/* I2C */
#define GY30_ADDR 0x23 /* Address of GY30 light sensor */
#define GY30_CONT_HIGH_RES_MODE 0x10 /* Start measurement at 11x resolution. Measurement time is approx 120mx */
#define GY30_CONSTANT_NUM (1.2)

static struct {
	int opened;
	peripheral_i2c_h sensor_h;
} resource_sensor_s;

void resource_close_illuminance_sensor(void)
{
	if (!resource_sensor_s.opened) return;

	_I("Illuminance Sensor is finishing...");
	peripheral_i2c_close(resource_sensor_s.sensor_h);
	resource_sensor_s.opened = 0;
}

int resource_read_illuminance_sensor(int i2c_bus, uint32_t *out_value)
{
	int ret = PERIPHERAL_ERROR_NONE;
	unsigned char buf[10] = { 0, };

	if (!resource_sensor_s.opened) {
		ret = peripheral_i2c_open(i2c_bus, GY30_ADDR, &resource_sensor_s.sensor_h);
		retv_if(!resource_sensor_s.sensor_h, -1);
		resource_sensor_s.opened = 1;
	}

	buf[0] = GY30_CONT_HIGH_RES_MODE;
	ret = peripheral_i2c_write(resource_sensor_s.sensor_h, buf, 1);
	retv_if(ret < 0, -1);

	ret = peripheral_i2c_read(resource_sensor_s.sensor_h, buf, 2);
	retv_if(ret < 0, -1);

	*out_value = (buf[0] << 8 | buf[1]) / GY30_CONSTANT_NUM; // Just Sum High 8bit and Low 8bit

	_I("Illuminance Sensor Value : %d", *out_value);

	return 0;
}
