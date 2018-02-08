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

#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <peripheral_io.h>
#include "log.h"
#include "resource/resource_PCA9685.h"

#define RPI3_I2C_BUS 1

/* Registers/etc: */
#define PCA9685_ADDRESS    0x40
#define MODE1              0x00
#define MODE2              0x01
#define SUBADR1            0x02
#define SUBADR2            0x03
#define SUBADR3            0x04
#define PRESCALE           0xFE
#define LED0_ON_L          0x06
#define LED0_ON_H          0x07
#define LED0_OFF_L         0x08
#define LED0_OFF_H         0x09
#define ALL_LED_ON_L       0xFA
#define ALL_LED_ON_H       0xFB
#define ALL_LED_OFF_L      0xFC
#define ALL_LED_OFF_H      0xFD

/* Bits: */
#define RESTART            0x80
#define SLEEP              0x10
#define ALLCALL            0x01
#define INVRT              0x10
#define OUTDRV             0x04

typedef enum {
	PCA9685_CH_STATE_NONE,
	PCA9685_CH_STATE_USED,
} pca9685_ch_state_e;

static peripheral_i2c_h g_i2c_h = NULL;
static unsigned int ref_count = 0;
static pca9685_ch_state_e ch_state[PCA9685_CH_MAX + 1] = {PCA9685_CH_STATE_NONE, };

int resource_pca9685_set_frequency(unsigned int freq_hz)
{
	int ret = PERIPHERAL_ERROR_NONE;
	double prescale_value = 0.0;
	int prescale = 0;
	uint8_t oldmode = 0;
	uint8_t newmode = 0;

	prescale_value = 25000000.0;	// 25MHz
	prescale_value /= 4096.0;	// 12-bit
	prescale_value /= (double)freq_hz;
	prescale_value -= 1.0;

	prescale = (int)floor(prescale_value + 0.5);

	ret = peripheral_i2c_read_register_byte(g_i2c_h, MODE1, &oldmode);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to read register");

	newmode = (oldmode & 0x7F) | 0x10; // sleep
	ret = peripheral_i2c_write_register_byte(g_i2c_h, MODE1, newmode); // go to sleep
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h, PRESCALE, prescale);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h, MODE1, oldmode);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	usleep(500);

	ret = peripheral_i2c_write_register_byte(g_i2c_h, MODE1, (oldmode | 0x80));
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	return 0;
}

int resource_pca9685_set_value_to_channel(unsigned int channel, int on, int off)
{
	int ret = PERIPHERAL_ERROR_NONE;
	retvm_if(g_i2c_h == NULL, -1, "Not initialized yet");

	retvm_if(ch_state[channel] == PCA9685_CH_STATE_NONE, -1,
		"ch[%u] is not in used state", channel);

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
			LED0_ON_L + 4*channel, on & 0xFF);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
			LED0_ON_H + 4*channel, on >> 8);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
			LED0_OFF_L + 4*channel, off & 0xFF);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
			LED0_OFF_H + 4*channel, off >> 8);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	return 0;
}

static int resource_pca9685_set_value_to_all(int on, int off)
{
	int ret = PERIPHERAL_ERROR_NONE;
	retvm_if(g_i2c_h == NULL, -1, "Not initialized yet");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
		ALL_LED_ON_L, on & 0xFF);

	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
		ALL_LED_ON_H, on >> 8);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
		ALL_LED_OFF_L, off & 0xFF);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	ret = peripheral_i2c_write_register_byte(g_i2c_h,
		ALL_LED_OFF_H, off >> 8);
	retvm_if(ret != PERIPHERAL_ERROR_NONE, -1, "failed to write register");

	return 0;
}

int resource_pca9685_init(unsigned int ch)
{
	uint8_t mode1 = 0;
	int ret = PERIPHERAL_ERROR_NONE;

	if (ch > PCA9685_CH_MAX) {
		_E("channel[%u] is out of range", ch);
		return -1;
	}

	if (ch_state[ch] == PCA9685_CH_STATE_USED) {
		_E("channel[%u] is already in used state", ch);
		return -1;
	}

	if (g_i2c_h)
		goto PASS_OPEN_HANDLE;

	ret = peripheral_i2c_open(RPI3_I2C_BUS, PCA9685_ADDRESS, &g_i2c_h);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to open pca9685-[bus:%d, addr:%d]",
			RPI3_I2C_BUS, PCA9685_ADDRESS);
		return -1;
	}
	ret = resource_pca9685_set_value_to_all(0, 0);
	if (ret) {
		_E("failed to reset all value to register");
		goto ERROR;
	}

	ret = peripheral_i2c_write_register_byte(g_i2c_h, MODE2, OUTDRV);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to write register");
		goto ERROR;
	}

	ret = peripheral_i2c_write_register_byte(g_i2c_h, MODE1, ALLCALL);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to write register");
		goto ERROR;
	}

	usleep(500); // wait for oscillator

	ret = peripheral_i2c_read_register_byte(g_i2c_h, MODE1, &mode1);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to read register");
		goto ERROR;
	}

	mode1 = mode1 & (~SLEEP); // # wake up (reset sleep)
	ret = peripheral_i2c_write_register_byte(g_i2c_h, MODE1, mode1);
	if (ret != PERIPHERAL_ERROR_NONE) {
		_E("failed to write register");
		goto ERROR;
	}

	usleep(500); // wait for oscillator

	ret = resource_pca9685_set_frequency(60);
	if (ret) {
		_E("failed to set frequency");
		goto ERROR;
	}

PASS_OPEN_HANDLE:
	ref_count++;
	ch_state[ch] = PCA9685_CH_STATE_USED;
	_D("pca9685 - ref_count[%u]", ref_count);
	_D("sets ch[%u] used state", ch);

	return 0;

ERROR:
	if (g_i2c_h)
		peripheral_i2c_close(g_i2c_h);

	g_i2c_h = NULL;
	return -1;
}

int resource_pca9685_fini(unsigned int ch)
{
	if (ch_state[ch] == PCA9685_CH_STATE_NONE) {
		_E("channel[%u] is not in used state", ch);
		return -1;
	}
	resource_pca9685_set_value_to_channel(ch, 0, 0);
	ch_state[ch] = PCA9685_CH_STATE_NONE;

	ref_count--;
	_D("ref count - %u", ref_count);

	if (ref_count == 0 && g_i2c_h) {
		_D("finalizing pca9685");
		resource_pca9685_set_value_to_all(0, 0);
		peripheral_i2c_close(g_i2c_h);
		g_i2c_h = NULL;
	}

	return 0;
}
