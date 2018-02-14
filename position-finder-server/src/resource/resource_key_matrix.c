#include <stdlib.h>
#include <unistd.h>
#include <peripheral_io.h>

#include "log.h"
#include "resource_key_matrix.h"
#include "resource_internal.h"

typedef enum{
	KEY_MATRIX_READY,
	KEY_MATRIX_CLOSE
} key_matrix_state_e;

typedef struct __key_matrix_s{
	unsigned int row_pin1;
	unsigned int row_pin2;
	unsigned int row_pin3;
	unsigned int row_pin4;
	unsigned int col_pin1;
	unsigned int col_pin2;
	unsigned int col_pin3;
	unsigned int col_pin4;
	peripheral_gpio_h row_pin1_h;
	peripheral_gpio_h row_pin2_h;
	peripheral_gpio_h row_pin3_h;
	peripheral_gpio_h row_pin4_h;
	peripheral_gpio_h col_pin1_h;
	peripheral_gpio_h col_pin2_h;
	peripheral_gpio_h col_pin3_h;
	peripheral_gpio_h col_pin4_h;
	key_matrix_state_e key_matrix_state;

} key_matrix_s;

static key_matrix_s g_km_h = {0, 0, 0, 0, 0, 0, 0, 0,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, KEY_MATRIX_CLOSE};

void resource_close_key_matrix()
{
	if (g_km_h.row_pin1_h) {
		peripheral_gpio_close(g_km_h.row_pin1_h);
		g_km_h.row_pin1_h = NULL;
	}

	if (g_km_h.row_pin2_h) {
		peripheral_gpio_close(g_km_h.row_pin2_h);
		g_km_h.row_pin2_h = NULL;
	}

	if (g_km_h.row_pin3_h) {
		peripheral_gpio_close(g_km_h.row_pin3_h);
		g_km_h.row_pin3_h = NULL;
	}

	if (g_km_h.row_pin4_h) {
		peripheral_gpio_close(g_km_h.row_pin4_h);
		g_km_h.row_pin4_h = NULL;
	}

	if (g_km_h.col_pin1_h) {
	peripheral_gpio_close(g_km_h.col_pin1_h);
		g_km_h.col_pin1_h = NULL;
	}

	if (g_km_h.col_pin2_h) {
		peripheral_gpio_close(g_km_h.col_pin2_h);
		g_km_h.col_pin2_h = NULL;
	}

	if (g_km_h.col_pin3_h) {
		peripheral_gpio_close(g_km_h.col_pin3_h);
		g_km_h.col_pin3_h = NULL;
	}

	if (g_km_h.col_pin4_h) {
		peripheral_gpio_close(g_km_h.col_pin4_h);
		g_km_h.col_pin4_h = NULL;
	}

	g_km_h.key_matrix_state = KEY_MATRIX_CLOSE;
}

static int __set_default_gpio_by_key_matrix()
{
	g_km_h.row_pin1 = DEFAULT_KEY_MATRIX_ROW1;
	g_km_h.row_pin2 = DEFAULT_KEY_MATRIX_ROW2;
	g_km_h.row_pin3 = DEFAULT_KEY_MATRIX_ROW3;
	g_km_h.row_pin4 = DEFAULT_KEY_MATRIX_ROW4;
	g_km_h.col_pin1 = DEFAULT_KEY_MATRIX_COL1;
	g_km_h.col_pin2 = DEFAULT_KEY_MATRIX_COL2;
	g_km_h.col_pin3 = DEFAULT_KEY_MATRIX_COL3;
	g_km_h.col_pin4 = DEFAULT_KEY_MATRIX_COL4;

	return 0;
}

int resource_set_key_matrix_gpio(unsigned int row1, unsigned int row2, unsigned int row3, unsigned int row4,
		unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4)
{
	g_km_h.row_pin1 = row1;
	g_km_h.row_pin2 = row2;
	g_km_h.row_pin3 = row3;
	g_km_h.row_pin4 = row4;
	g_km_h.col_pin1 = col1;
	g_km_h.col_pin2 = col2;
	g_km_h.col_pin3 = col3;
	g_km_h.col_pin4 = col4;
	g_km_h.key_matrix_state = KEY_MATRIX_READY;
	return 0;
}

static int __init_key_matrix()
{
	int ret =0;

	if(g_km_h.key_matrix_state == KEY_MATRIX_CLOSE)
		__set_default_gpio_by_key_matrix();

	ret = peripheral_gpio_open(g_km_h.row_pin1, &g_km_h.row_pin1_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.row_pin1_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio row1[%u] pin", g_km_h.row_pin1);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.row_pin2, &g_km_h.row_pin2_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.row_pin2_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio row2[%u] pin", g_km_h.row_pin2);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.row_pin3, &g_km_h.row_pin3_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.row_pin3_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio row3[%u] pin", g_km_h.row_pin3);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.row_pin4, &g_km_h.row_pin4_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.row_pin4_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio row4[%u] pin", g_km_h.row_pin4);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.col_pin1, &g_km_h.col_pin1_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.col_pin1_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio col1[%u] pin", g_km_h.col_pin1);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.col_pin2, &g_km_h.col_pin2_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.col_pin2_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio col2[%u] pin", g_km_h.col_pin2);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.col_pin3, &g_km_h.col_pin3_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.col_pin3_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio col3[%u] pin", g_km_h.col_pin3);
		goto ERROR;
	}

	ret = peripheral_gpio_open(g_km_h.col_pin4, &g_km_h.col_pin4_h);
	if (ret == PERIPHERAL_ERROR_NONE)
		peripheral_gpio_set_direction(g_km_h.col_pin4_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	else {
		_E("failed to open gpio col4[%u] pin", g_km_h.col_pin4);
		goto ERROR;
	}

	g_km_h.key_matrix_state = KEY_MATRIX_READY;

	return 0;

ERROR:
	if (g_km_h.row_pin1_h) {
		peripheral_gpio_close(g_km_h.row_pin1_h);
		g_km_h.row_pin1_h = NULL;
	}

	if (g_km_h.row_pin2_h) {
		peripheral_gpio_close(g_km_h.row_pin2_h);
		g_km_h.row_pin2_h = NULL;
	}

	if (g_km_h.row_pin3_h) {
		peripheral_gpio_close(g_km_h.row_pin3_h);
		g_km_h.row_pin3_h = NULL;
	}

	if (g_km_h.row_pin4_h) {
		peripheral_gpio_close(g_km_h.row_pin4_h);
		g_km_h.row_pin4_h = NULL;
	}

	if (g_km_h.col_pin1_h) {
	peripheral_gpio_close(g_km_h.col_pin1_h);
		g_km_h.col_pin1_h = NULL;
	}

	if (g_km_h.col_pin2_h) {
		peripheral_gpio_close(g_km_h.col_pin2_h);
		g_km_h.col_pin2_h = NULL;
	}

	if (g_km_h.col_pin3_h) {
		peripheral_gpio_close(g_km_h.col_pin3_h);
		g_km_h.col_pin3_h = NULL;
	}

	if (g_km_h.col_pin4_h) {
		peripheral_gpio_close(g_km_h.col_pin4_h);
		g_km_h.col_pin4_h = NULL;
	}
	return -1;
}

int resource_read_key_matrix()
{
	if(g_km_h.key_matrix_state == KEY_MATRIX_CLOSE)
		__init_key_matrix();


	return 0;
}
