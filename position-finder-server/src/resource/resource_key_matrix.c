#include <stdlib.h>
#include <unistd.h>
#include <peripheral_io.h>
#include <sys/time.h>

#include "log.h"
#include "resource/resource_key_matrix.h"
#include "resource_internal.h"

typedef enum{
	KEY_MATRIX_READY,
	KEY_MATRIX_CONFIGURED,
	KEY_MATRIX_CLOSE
} key_matrix_state_e;

typedef struct __key_matrix_s{
	unsigned int row_pin[4];
	unsigned int col_pin[4];
	peripheral_gpio_h row_pin_h[4];
	peripheral_gpio_h col_pin_h[4];
	key_matrix_state_e key_matrix_state;

} key_matrix_s;

static key_matrix_s g_km_h = {{0, 0, 0, 0}, {0, 0, 0, 0},
		{NULL, NULL, NULL, NULL}, {NULL, NULL, NULL, NULL}, KEY_MATRIX_CLOSE};

int row_num[4] = {0, 1, 2, 3};
int col_num[4] = {0, 1, 2, 3};
int push_row = -1;
int push_col = -1;

void resource_close_key_matrix()
{
	int i=0;
	for(i = 0; i < 4; i++){
		if(g_km_h.row_pin_h[i]){
			peripheral_gpio_unset_interrupted_cb(g_km_h.row_pin_h[i]);
			peripheral_gpio_close(g_km_h.row_pin_h[i]);
			g_km_h.row_pin_h[i] = NULL;
		}
		if(g_km_h.col_pin_h[i]){
			peripheral_gpio_close(g_km_h.col_pin_h[i]);
			g_km_h.col_pin_h[i] = NULL;
		}
	}

	g_km_h.key_matrix_state = KEY_MATRIX_CLOSE;
}

static int __set_default_gpio_by_key_matrix()
{
	g_km_h.row_pin[0] = DEFAULT_KEY_MATRIX_ROW1;
	g_km_h.row_pin[1] = DEFAULT_KEY_MATRIX_ROW2;
	g_km_h.row_pin[2] = DEFAULT_KEY_MATRIX_ROW3;
	g_km_h.row_pin[3] = DEFAULT_KEY_MATRIX_ROW4;
	g_km_h.col_pin[0] = DEFAULT_KEY_MATRIX_COL1;
	g_km_h.col_pin[1] = DEFAULT_KEY_MATRIX_COL2;
	g_km_h.col_pin[2] = DEFAULT_KEY_MATRIX_COL3;
	g_km_h.col_pin[3] = DEFAULT_KEY_MATRIX_COL4;

	return 0;
}

//static void _resource_read_key_matrix_cb(peripheral_gpio_h gpio, peripheral_error_e error, void *user_data)
//{
//	int row_num = *((int*)(user_data));
//	push_row = row_num;
//	_D("click push_row[%d]", push_row);
//}
//
//static void _resource_read_key_matrix_cb2(peripheral_gpio_h gpio, peripheral_error_e error, void *user_data)
//{
//	int col_num = *((int*)(user_data));
//	push_col = col_num;
//	_D("click push_col[%d]", push_col);
//}

static int __init_key_matrix()
{
	int ret = 0;
	int i = 0;
	if(g_km_h.key_matrix_state == KEY_MATRIX_CLOSE)
		__set_default_gpio_by_key_matrix();

	for(i = 0 ; i < 4 ; i++){
		ret = peripheral_gpio_open(g_km_h.row_pin[i], &g_km_h.row_pin_h[i]);
		if (ret == PERIPHERAL_ERROR_NONE){
//			peripheral_gpio_set_direction(g_km_h.row_pin_h[i],
//						PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_HIGH);
			peripheral_gpio_set_direction(g_km_h.row_pin_h[i],
				PERIPHERAL_GPIO_DIRECTION_IN);
//			peripheral_gpio_set_edge_mode(g_km_h.row_pin_h[i], PERIPHERAL_GPIO_EDGE_BOTH);
//			peripheral_gpio_set_interrupted_cb(g_km_h.row_pin_h[i], _resource_read_key_matrix_cb, (void*)&row_num[i]);
		}
		else {
			_E("failed to open gpio row%d[%u] pin", i, g_km_h.row_pin[i]);
			goto ERROR;
		}

		ret = peripheral_gpio_open(g_km_h.col_pin[i], &g_km_h.col_pin_h[i]);
		if (ret == PERIPHERAL_ERROR_NONE){
			peripheral_gpio_set_direction(g_km_h.col_pin_h[i],
				PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_HIGH);
//			peripheral_gpio_set_direction(g_km_h.col_pin_h[i],
//						PERIPHERAL_GPIO_DIRECTION_IN);
//			peripheral_gpio_set_edge_mode(g_km_h.col_pin_h[i], PERIPHERAL_GPIO_EDGE_BOTH);
//			peripheral_gpio_set_interrupted_cb(g_km_h.col_pin_h[i], _resource_read_key_matrix_cb2, (void*)&col_num[i]);
		}
		else {
			_E("failed to open gpio col%d[%u] pin", i, g_km_h.col_pin[i]);
			goto ERROR;
		}
	}

	g_km_h.key_matrix_state = KEY_MATRIX_READY;

	return 0;

ERROR:

	for(i = 0; i < 4; i++){
		if(g_km_h.row_pin_h[i]){
			peripheral_gpio_unset_interrupted_cb(g_km_h.row_pin_h[i]);
			peripheral_gpio_close(g_km_h.row_pin_h[i]);
			g_km_h.row_pin_h[i] = NULL;
		}
		if(g_km_h.col_pin_h[i]){
			peripheral_gpio_close(g_km_h.col_pin_h[i]);
			g_km_h.col_pin_h[i] = NULL;
		}
	}

	return -1;
}

int resource_set_key_matrix_gpio(unsigned int row1, unsigned int row2, unsigned int row3, unsigned int row4,
		unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4)
{
	if(g_km_h.key_matrix_state == KEY_MATRIX_READY)
		resource_close_key_matrix();

	g_km_h.row_pin[0] = row1;
	g_km_h.row_pin[1] = row2;
	g_km_h.row_pin[2] = row3;
	g_km_h.row_pin[3] = row4;
	g_km_h.col_pin[0] = col1;
	g_km_h.col_pin[1] = col2;
	g_km_h.col_pin[2] = col3;
	g_km_h.col_pin[3] = col4;
	g_km_h.key_matrix_state = KEY_MATRIX_CONFIGURED;

	__init_key_matrix();
	return 0;
}

int resource_read_key_matrix()
{
	uint32_t read;
	int i;
	int j;
	int ret = 0;
	if(g_km_h.key_matrix_state != KEY_MATRIX_READY)
		__init_key_matrix();

//	if(push_col != -1 && push_row != -1)
//		_D("press [%c] key!!", key_matrix_value[push_row][push_col]);
//
//	push_col = push_row = -1;

	for(i = 0; i < 4; i++){
		ret = peripheral_gpio_write(g_km_h.col_pin_h[i], 0);
		if(ret != PERIPHERAL_ERROR_NONE){
			_E("failed to set value[0] col[%d] pin", i);
			ret = -1;
			return ret;
		}

		for(j = 0; j < 4; j++){
			peripheral_gpio_read(g_km_h.row_pin_h[j], &read);
			if(read == 0){
				_D("press [%c] key!!", key_matrix_value[j][i]);
				do{
					peripheral_gpio_read(g_km_h.row_pin_h[j], &read);
				}while(read == 0);
			}
		}

		ret = peripheral_gpio_write(g_km_h.col_pin_h[i], 1);
		if(ret != PERIPHERAL_ERROR_NONE){
			_E("failed to set value[1] col[%d] pin", i);
			ret = -1;
			return ret;
		}
	}

	return 0;
}
