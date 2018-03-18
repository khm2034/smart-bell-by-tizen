#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <peripheral_io.h>
#include "log.h"
#include "resource/resource_1602A_LCD.h"
#include "smartbell/smartbell_util.h"

#define TRUE (1==1)
#define FALSE (1==2)

// HD44780U Commands

#define	LCD_CLEAR	0x01
#define	LCD_HOME	0x02
#define	LCD_ENTRY	0x04
#define	LCD_CTRL	0x08
#define	LCD_CDSHIFT	0x10
#define	LCD_FUNC	0x20
#define	LCD_CGRAM	0x40
#define	LCD_DGRAM	0x80

// Bits in the entry register

#define	LCD_ENTRY_SH		0x01
#define	LCD_ENTRY_ID		0x02

// Bits in the control register

#define	LCD_BLINK_CTRL		0x01
#define	LCD_CURSOR_CTRL		0x02
#define	LCD_DISPLAY_CTRL	0x04

#define	LCD_FUNC_F	0x04
#define	LCD_FUNC_N	0x08
#define	LCD_FUNC_DL	0x10

#define	LCD_CDSHIFT_RL	0x04

typedef struct __lcd_data{
	int bits, rows, cols ;
	unsigned int rs_pin, strb_pin ;
	unsigned int data_pins [8] ;
	peripheral_gpio_h rs_pin_h;
	peripheral_gpio_h strb_pin_h;
	peripheral_gpio_h data_pins_h[8];
	int cx, cy ;
} lcd_data;

static lcd_data *lcds [MAX_LCDS] ;
static int lcd_control ;
static const int row_off [4] = { 0x00, 0x40, 0x14, 0x54 } ;

void resource_1602A_LCD_close(){
	int i, j;
	for(i = 0; i< MAX_LCDS; i++){
		if(lcds[i]->rs_pin_h){
			peripheral_gpio_close(lcds[i]->rs_pin_h);
			lcds[i]->rs_pin_h = NULL;
		}

		if(lcds[i]->strb_pin_h){
			peripheral_gpio_close(lcds[i]->strb_pin_h);
			lcds[i]->strb_pin_h = NULL;
		}

		for (j = 0 ; j < lcds[i]->bits ; ++j){
			if(lcds[i]->data_pins_h[j]){
				peripheral_gpio_close(lcds[i]->data_pins_h[j]);
				lcds[i]->data_pins_h[j] = NULL;
			}
		}
		free(lcds[i]);
		lcds[i] = NULL;
	}
}

static int _resource_1602A_LCD_strobe (const lcd_data *lcd){
	int ret = 0;
	ret = peripheral_gpio_write(lcd->strb_pin_h, 1);
	if(ret != PERIPHERAL_ERROR_NONE){
		_E("failed to set value[1] strb pin");
		ret = -1;
		return ret;
	}
	delay_microseconds (50) ;
	ret = peripheral_gpio_write(lcd->strb_pin_h, 0);
	if(ret != PERIPHERAL_ERROR_NONE){
		_E("failed to set value[0] strb pin");
		ret = -1;
		return ret;
	}
	delay_microseconds (50) ;
	return ret;
}

static int _resource_1602A_LCD_send_data_cmd (const lcd_data *lcd, unsigned char data){
	register unsigned char myData = data;
	unsigned char i, d4;
	int ret = 0;
	int tmp;
	peripheral_gpio_read(lcd->rs_pin_h, &tmp);
	if (lcd->bits == 4){
		d4 = (myData >> 4) & 0x0F;

		for (i = 0 ; i < 4 ; ++i){
			ret = peripheral_gpio_write(lcd->data_pins_h[i], (d4 & 1));
			if(ret != PERIPHERAL_ERROR_NONE){
				_E("failed to set value[0] col[%d] pin", i);
				ret = -1;
				return ret;
			}
			d4 >>= 1 ;
		}
		_resource_1602A_LCD_strobe(lcd) ;

		d4 = myData & 0x0F ;
		for (i = 0 ; i < 4 ; ++i){
			ret = peripheral_gpio_write(lcd->data_pins_h[i], (d4 & 1));
			if(ret != PERIPHERAL_ERROR_NONE){
				_E("failed to set value[0] col[%d] pin", i);
				ret = -1;
				return ret;
			}
			d4 >>= 1 ;
		}
	}
	else{
		for (i = 0 ; i < 8 ; ++i){
			ret = peripheral_gpio_write(lcd->data_pins_h[i], (myData & 1));
			if(ret != PERIPHERAL_ERROR_NONE){
				_E("failed to set value[0] col[%d] pin", i);
				ret = -1;
				return ret;
			}
			myData >>= 1 ;
		}
	}
	ret = _resource_1602A_LCD_strobe(lcd) ;
	return ret;
}

static int _resource_1602A_LCD_put_command (const lcd_data *lcd, unsigned char command){
	int ret = 0;
	ret = peripheral_gpio_write(lcd->rs_pin_h, 0);
	if(ret != PERIPHERAL_ERROR_NONE){
		_E("failed to set value[0] rs pin");
		ret = -1;
		return ret;
	}
	ret = _resource_1602A_LCD_send_data_cmd(lcd, command);
	delay (2);
	return ret;
}

static int _resource_1602A_LCD_put_4_command (const lcd_data *lcd, unsigned char command){
	register unsigned char myCommand = command ;
	register unsigned char i ;
	int ret = 0;

	ret = peripheral_gpio_write(lcd->rs_pin_h, 0);
	if(ret != PERIPHERAL_ERROR_NONE){
		_E("failed to set value[0] col[%d] pin", i);
		ret = -1;
		return ret;
	}

	for (i = 0 ; i < 4 ; ++i){
		ret = peripheral_gpio_write(lcd->data_pins_h[i], (myCommand & 1));
		if(ret != PERIPHERAL_ERROR_NONE){
			_E("failed to set value[0] col[%d] pin", i);
			ret = -1;
			return ret;
		}
		myCommand >>= 1 ;
	}
	ret = _resource_1602A_LCD_strobe(lcd);
	return ret;
}

void __resource_1602A_LCD_home (const int fd){
	lcd_data *lcd = lcds [fd] ;

	_resource_1602A_LCD_put_command(lcd, LCD_HOME) ;
	lcd->cx = lcd->cy = 0 ;
	delay (5) ;
}

void __resource_1602A_LCD_clear (const int fd){
	lcd_data *lcd = lcds [fd] ;

	_resource_1602A_LCD_put_command(lcd, LCD_CLEAR) ;
	_resource_1602A_LCD_put_command(lcd, LCD_HOME) ;
	lcd->cx = lcd->cy = 0 ;
	delay (5) ;
}

void __resource_1602A_LCD_display (const int fd, int state){
	lcd_data *lcd = lcds[fd] ;

	if (state)
		lcd_control |=  LCD_DISPLAY_CTRL ;
	else
		lcd_control &= ~LCD_DISPLAY_CTRL ;

	_resource_1602A_LCD_put_command(lcd, LCD_CTRL | lcd_control) ;
}

void __resource_1602A_LCD_cursor (const int fd, int state){
	lcd_data *lcd = lcds [fd] ;

	if (state)
		lcd_control |=  LCD_CURSOR_CTRL ;
	else
		lcd_control &= ~LCD_CURSOR_CTRL ;

	_resource_1602A_LCD_put_command(lcd, LCD_CTRL | lcd_control) ;
}

void __resource_1602A_LCD_cursor_blink (const int fd, int state){
	lcd_data *lcd = lcds [fd] ;

	if (state)
		lcd_control |=  LCD_BLINK_CTRL ;
	else
		lcd_control &= ~LCD_BLINK_CTRL ;

	_resource_1602A_LCD_put_command(lcd, LCD_CTRL | lcd_control) ;
}

void __resource_1602A_LCD_send_command (const int fd, unsigned char command){
	lcd_data *lcd = lcds [fd] ;
	_resource_1602A_LCD_put_command(lcd, command) ;
}

void resource_1602A_LCD_position (const int fd, int x, int y)
{
	lcd_data *lcd = lcds [fd] ;

	if ((x > lcd->cols) || (x < 0))
		return ;
	if ((y > lcd->rows) || (y < 0))
		return ;

	_resource_1602A_LCD_put_command(lcd, x + (LCD_DGRAM | row_off[y])) ;

	lcd->cx = x ;
	lcd->cy = y ;
}

int resource_1602A_LCD_char_def(const int fd, int index, unsigned char data [8]){
	lcd_data *lcd = lcds [fd] ;
	int i;
	int ret = 0;
	_resource_1602A_LCD_put_command(lcd, LCD_CGRAM | ((index & 7) << 3)) ;

	ret = peripheral_gpio_write(lcd->rs_pin_h, 1);
	if(ret != PERIPHERAL_ERROR_NONE){
		_E("failed to set value[0] rs pin", i);
		ret = -1;
		return ret;
	}
	for (i = 0 ; i < 8 ; ++i)
		_resource_1602A_LCD_send_data_cmd(lcd, data[i]) ;
	return 0;
}

int resource_1602A_LCD_putchar (const int fd, unsigned char data){
	lcd_data *lcd = lcds [fd] ;
	int ret = 0;
	_D("data : %c", data);
	ret = peripheral_gpio_write(lcd->rs_pin_h, 1);
	if(ret != PERIPHERAL_ERROR_NONE){
		_E("failed to set value[0] rs pin");
		ret = -1;
		return ret;
	}
	ret = _resource_1602A_LCD_send_data_cmd(lcd, data) ;

	if (++lcd->cx == lcd->cols){
		lcd->cx = 0 ;
		if (++lcd->cy == lcd->rows)
			lcd->cy = 0 ;
		_resource_1602A_LCD_put_command(lcd, lcd->cx + (LCD_DGRAM | row_off [lcd->cy])) ;
	}
	return ret;
}

void resource_1602A_LCD_puts (const int fd, const char *string){
	while (*string)
		resource_1602A_LCD_putchar(fd, *string++) ;
}

void resource_1602A_LCD_printf (const int fd, const char *message, ...)
{
	va_list argp ;
	char buffer [1024] ;

	va_start (argp, message) ;
	vsnprintf (buffer, 1023, message, argp) ;
	va_end (argp) ;

	resource_1602A_LCD_puts(fd, buffer) ;
}

int resource_1602A_LCD_init (const int rows, const int cols, const int bits,
	const int rs, const int strb,
	const int d0, const int d1, const int d2, const int d3, const int d4,
	const int d5, const int d6, const int d7){
	int ret = 0;

	unsigned char func ;
	int i ;
	int lcd_fd = -1 ;
	lcd_data *lcd ;

	static int init = 0 ;

	if (init == 0){
		init = 1 ;
		for (i = 0 ; i < MAX_LCDS ; ++i)
		  lcds [i] = NULL ;
	}

	if (! ((bits == 4) || (bits == 8)))
		return -1 ;

	if ((rows < 0) || (rows > 20))
		return -1 ;

	if ((cols < 0) || (cols > 20))
		return -1 ;

	for (i = 0 ; i < MAX_LCDS ; ++i){
		if (lcds [i] == NULL){
			lcd_fd = i;
			break ;
		}
	}

	if (lcd_fd == -1)
		return -1 ;

	lcd = (lcd_data *)malloc (sizeof (lcd_data)) ;
	if (lcd == NULL)
		return -1 ;

	lcd->rs_pin   = rs ;
	lcd->strb_pin = strb ;
	lcd->bits    = 8 ;
	lcd->rows    = rows ;
	lcd->cols    = cols ;
	lcd->cx      = 0 ;
	lcd->cy      = 0 ;

	lcd->data_pins[0] = d0 ;
	lcd->data_pins[1] = d1 ;
	lcd->data_pins[2] = d2 ;
	lcd->data_pins[3] = d3 ;
	lcd->data_pins[4] = d4 ;
	lcd->data_pins[5] = d5 ;
	lcd->data_pins[6] = d6 ;
	lcd->data_pins[7] = d7 ;
	lcds [lcd_fd] = lcd ;

	ret = peripheral_gpio_open(lcd->rs_pin, &lcd->rs_pin_h);
	if (ret == PERIPHERAL_ERROR_NONE){
		peripheral_gpio_set_direction(lcd->rs_pin_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	}
	else {
		_E("failed to open gpio rs pin[%u]", lcd->rs_pin);
		goto ERROR;
	}

	ret = peripheral_gpio_open(lcd->strb_pin, &lcd->strb_pin_h);
	if (ret == PERIPHERAL_ERROR_NONE){
		peripheral_gpio_set_direction(lcd->strb_pin_h,
			PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
	}
	else {
		_E("failed to open gpio strb pin[%u]", lcd->strb_pin_h);
		goto ERROR;
	}

	for (i = 0 ; i < bits ; ++i){
		ret = peripheral_gpio_open(lcd->data_pins[i], &lcd->data_pins_h[i]);
		if (ret == PERIPHERAL_ERROR_NONE){
			peripheral_gpio_set_direction(lcd->data_pins_h[i],
				PERIPHERAL_GPIO_DIRECTION_OUT_INITIALLY_LOW);
		}
		else {
			_E("failed to open gpio data[%d] pin[%u]", i, lcd->data_pins[i]);
			goto ERROR;
		}
	}
	delay (35) ; // mS

	if (bits == 4){
		func = LCD_FUNC | LCD_FUNC_DL ;			// Set 8-bit mode 3 times
		_resource_1602A_LCD_put_4_command(lcd, func >> 4) ; delay (35) ;
		_resource_1602A_LCD_put_4_command(lcd, func >> 4) ; delay (35) ;
		_resource_1602A_LCD_put_4_command(lcd, func >> 4) ; delay (35) ;
		func = LCD_FUNC ;					// 4th set: 4-bit mode
		_resource_1602A_LCD_put_4_command(lcd, func >> 4) ; delay (35) ;
		lcd->bits = 4 ;
	}

	else{
		func = LCD_FUNC | LCD_FUNC_DL ;
		_resource_1602A_LCD_put_command(lcd, func) ; delay (35) ;
		_resource_1602A_LCD_put_command(lcd, func) ; delay (35) ;
		_resource_1602A_LCD_put_command(lcd, func) ; delay (35) ;
	}

	if (lcd->rows > 1){
		func |= LCD_FUNC_N ;
		_resource_1602A_LCD_put_command(lcd, func) ; delay (35) ;
	}

	__resource_1602A_LCD_display(lcd_fd, TRUE) ;
	__resource_1602A_LCD_cursor(lcd_fd, FALSE) ;
	__resource_1602A_LCD_cursor_blink(lcd_fd, FALSE) ;
	__resource_1602A_LCD_clear(lcd_fd) ;

	_resource_1602A_LCD_put_command(lcd, LCD_ENTRY   | LCD_ENTRY_ID) ;
	_resource_1602A_LCD_put_command(lcd, LCD_CDSHIFT | LCD_CDSHIFT_RL) ;

	return lcd_fd;

ERROR:
	if(lcd->rs_pin_h){
		peripheral_gpio_close(lcd->rs_pin_h);
		lcd->rs_pin_h = NULL;
	}

	if(lcd->strb_pin_h){
		peripheral_gpio_close(lcd->strb_pin_h);
		lcd->strb_pin_h = NULL;
	}

	for (i = 0 ; i < bits ; ++i){
		if(lcd->data_pins_h[i]){
			peripheral_gpio_close(lcd->data_pins_h[i]);
			lcd->data_pins_h[i] = NULL;
		}
	}

	return -1;
}
