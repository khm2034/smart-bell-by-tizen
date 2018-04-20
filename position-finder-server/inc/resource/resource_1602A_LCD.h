
#ifndef __POSITION_FINDER_RESOURCE_1602A_resource_1602A_LCD__H__
#define __POSITION_FINDER_RESOURCE_1602A_resource_1602A_LCD__H__

typedef enum {
	LCD_ID_1,
	LCD_MAX
} lcd_id_e;

#define DEFAULT_ROW 2
#define DEFAULT_COL 16
#define DEFAULT_BITS 8
#define DEFAULT_RS_PIN 27
#define DEFAULT_STRB_PIN 22
#define DEFAULT_D0_PIN 6
#define DEFAULT_D1_PIN 13
#define DEFAULT_D2_PIN 19
#define DEFAULT_D3_PIN 26
#define DEFAULT_D4_PIN 12
#define DEFAULT_D5_PIN 16
#define DEFAULT_D6_PIN 20
#define DEFAULT_D7_PIN 21

#define TRUE (1==1)
#define FALSE (1==2)

#define	LCD_CLEAR	0x01
#define	LCD_HOME	0x02
#define	LCD_ENTRY	0x04
#define	LCD_CTRL	0x08
#define	LCD_CDSHIFT	0x10
#define	LCD_FUNC	0x20
#define	LCD_CGRAM	0x40
#define	LCD_DGRAM	0x80

#define	LCD_ENTRY_SH		0x01
#define	LCD_ENTRY_ID		0x02

#define	LCD_BLINK_CTRL		0x01
#define	LCD_CURSOR_CTRL		0x02
#define	LCD_DISPLAY_CTRL	0x04

#define	LCD_FUNC_F	0x04
#define	LCD_FUNC_N	0x08
#define	LCD_FUNC_DL	0x10

#define	LCD_CDSHIFT_RL	0x04

int resource_1602A_LCD_position (const lcd_id_e id, int x, int y);
int resource_1602A_LCD_char_def(const lcd_id_e id, int index, unsigned char data [8]) ;
int resource_1602A_LCD_putchar(const lcd_id_e id, unsigned char data) ;
void resource_1602A_LCD_puts(const lcd_id_e id, const char *string) ;
void resource_1602A_LCD_printf(const lcd_id_e id, const char *message, ...) ;
int resource_set_1602A_LCD_configuration(lcd_id_e id, int rows, int cols, int bits,
		int rs, int strb, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7 );

#endif
