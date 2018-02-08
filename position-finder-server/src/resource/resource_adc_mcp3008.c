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
#include <tizen.h>
#include <system_info.h>
#include <string.h>
#include "log.h"


#define	MCP3008_SPEED 3600000
#define MCP3008_BPW 8

#define	MCP3008_TX_WORD1     0x01	/* 0b00000001 */
#define	MCP3008_TX_CH0 0x80	/* 0b10000000 */
#define	MCP3008_TX_CH1 0x90	/* 0b10010000 */
#define	MCP3008_TX_CH2 0xA0	/* 0b10100000 */
#define	MCP3008_TX_CH3 0xB0	/* 0b10110000 */
#define	MCP3008_TX_CH4 0xC0	/* 0b11000000 */
#define	MCP3008_TX_CH5 0xD0	/* 0b11010000 */
#define	MCP3008_TX_CH6 0xE0	/* 0b11100000 */
#define	MCP3008_TX_CH7 0xF0	/* 0b11110000 */
#define	MCP3008_TX_WORD3     0x00	/* 0b00000000 */

#define MCP3008_RX_WORD1_MASK 0x00	/* 0b00000000 */
#define MCP3008_RX_WORD2_NULL_BIT_MASK 0x04	/* 0b00000100 */
#define MCP3008_RX_WORD2_MASK 0x03	/* 0b00000011 */
#define MCP3008_RX_WORD3_MASK 0xFF	/* 0b11111111 */
#define UINT10_VALIDATION_MASK 0x3FF

#define MODEL_NAME_KEY "http://tizen.org/system/model_name"
#define MODEL_NAME_RPI3 "rpi3"
#define MODEL_NAME_ARTIK "artik"

static peripheral_spi_h MCP3008_H = NULL;
static unsigned int ref_count = 0;

int resource_adc_mcp3008_init(void)
{
	int ret = 0;
	int bus = -1;
	char *model_name = NULL;

	if (MCP3008_H) {
		_D("SPI device aleady initialized [ref_count : %u]", ref_count);
		ref_count++;
		return 0;
	}

	system_info_get_platform_string(MODEL_NAME_KEY, &model_name);
	if (!model_name) {
		_E("fail to get model name");
		return -1;
	}

	if (!strcmp(model_name, MODEL_NAME_RPI3)) {
		bus = 0;
	} else if (!strcmp(model_name, MODEL_NAME_ARTIK)) {
		bus = 2;
	} else {
		_E("unknown model name : %s", model_name);
		free(model_name);
		return -1;
	}
	free(model_name);
	model_name = NULL;

	ret = peripheral_spi_open(bus, 0, &MCP3008_H);
	if (PERIPHERAL_ERROR_NONE != ret) {
		_E("spi open failed :%s ", get_error_message(ret));
		return -1;
	}

	ret = peripheral_spi_set_mode(MCP3008_H, PERIPHERAL_SPI_MODE_0);
	if (PERIPHERAL_ERROR_NONE != ret) {
		_E("peripheral_spi_set_mode failed :%s ", get_error_message(ret));
		goto error_after_open;
	}
	ret = peripheral_spi_set_bit_order(MCP3008_H, PERIPHERAL_SPI_BIT_ORDER_MSB);
	if (PERIPHERAL_ERROR_NONE != ret) {
		_E("peripheral_spi_set_bit_order failed :%s ", get_error_message(ret));
		goto error_after_open;
	}

	ret = peripheral_spi_set_bits_per_word(MCP3008_H, MCP3008_BPW);
	if (PERIPHERAL_ERROR_NONE != ret) {
		_E("peripheral_spi_set_bits_per_word failed :%s ", get_error_message(ret));
		goto error_after_open;
	}

	ret = peripheral_spi_set_frequency(MCP3008_H, MCP3008_SPEED);
	if (PERIPHERAL_ERROR_NONE != ret) {
		_E("peripheral_spi_set_frequency failed :%s ", get_error_message(ret));
		goto error_after_open;
	}

	ref_count++;

	return 0;

error_after_open:
	peripheral_spi_close(MCP3008_H);
	MCP3008_H = NULL;
	return -1;
}


int resource_read_adc_mcp3008(int ch_num, unsigned int *out_value)
{
	unsigned char rx[3] = {0, };
	unsigned char tx[3] = {0, };
	unsigned char rx_w1 = 0;
	unsigned char rx_w2 = 0;
	unsigned char rx_w2_nb = 0;
	unsigned char rx_w3 = 0;
	unsigned short int result = 0;

	retv_if(MCP3008_H == NULL, -1);
	retv_if(out_value == NULL, -1);
	retv_if((ch_num < 0 || ch_num > 7), -1);

	tx[0] = MCP3008_TX_WORD1;
	switch (ch_num) {
	case 0:
		tx[1] = MCP3008_TX_CH0;
		break;
	case 1:
		tx[1] = MCP3008_TX_CH1;
		break;
	case 2:
		tx[1] = MCP3008_TX_CH2;
		break;
	case 3:
		tx[1] = MCP3008_TX_CH3;
		break;
	case 4:
		tx[1] = MCP3008_TX_CH4;
		break;
	case 5:
		tx[1] = MCP3008_TX_CH5;
		break;
	case 6:
		tx[1] = MCP3008_TX_CH6;
		break;
	case 7:
		tx[1] = MCP3008_TX_CH7;
		break;
	default:
		tx[1] = MCP3008_TX_CH0;
		break;
	}
	tx[2] = MCP3008_TX_WORD3;

	peripheral_spi_transfer(MCP3008_H, tx, rx, 3);

	rx_w1 = rx[0] & MCP3008_RX_WORD1_MASK;
	retv_if(rx_w1 != 0, -1);

	rx_w2_nb = rx[1] & MCP3008_RX_WORD2_NULL_BIT_MASK;
	retv_if(rx_w2_nb != 0, -1);

	rx_w2 = rx[1] & MCP3008_RX_WORD2_MASK;
	rx_w3 = rx[2] & MCP3008_RX_WORD3_MASK;

	result = ((rx_w2 << 8) | (rx_w3)) & UINT10_VALIDATION_MASK;

	// _D("%hu", result);

	*out_value = result;

	return 0;
}

void resource_adc_mcp3008_fini(void)
{
	if (MCP3008_H)
		ref_count--;
	else
		return;

	if (ref_count == 0) {
		peripheral_spi_close(MCP3008_H);
		MCP3008_H = NULL;
	}

	return;
}

