
#ifndef __POSITION_FINDER_RESOURCE_1602A_resource_1602A_LCD__H__
#define __POSITION_FINDER_RESOURCE_1602A_resource_1602A_LCD__H__

#define	MAX_LCDS 8
#define LCD_RS 5
#define LCD_E 24
#define LCD_D0 4
#define LCD_D1 17
#define LCD_D2 27
#define LCD_D3 22
#define LCD_D4 6
#define LCD_D5 13
#define LCD_D6 19
#define LCD_D7 26

void resource_1602A_LCD_home(const int fd) ;
void resource_1602A_LCD_clear(const int fd) ;
void resource_1602A_LCD_display(const int fd, int state) ;
void resource_1602A_LCD_cursor(const int fd, int state) ;
void resource_1602A_LCD_cursor_blink(const int fd, int state) ;
void resource_1602A_LCD_send_command(const int fd, unsigned char command) ;
void resource_1602A_LCD_position(const int fd, int x, int y) ;
int resource_1602A_LCD_char_def(const int fd, int index, unsigned char data [8]) ;
int resource_1602A_LCD_putchar(const int fd, unsigned char data) ;
void resource_1602A_LCD_puts(const int fd, const char *string) ;
void resource_1602A_LCD_printf(const int fd, const char *message, ...) ;

int  resource_1602A_LCD_init (const int rows, const int cols, const int bits,
	const int rs, const int strb,
	const int d0, const int d1, const int d2, const int d3, const int d4,
	const int d5, const int d6, const int d7) ;
//
//int digit_mapping[10][8] = {
//		{0,0,1,1,0,0,0,0}, {0,0,1,1,0,0,0,1},{0,0,1,1,0,0,1,0},
//		{0,0,1,1,0,0,1,1}, {0,0,1,1,0,1,0,0}, {0,0,1,1,0,1,0,1},
//		{0,0,1,1,0,1,1,0}, {0,0,1,1,0,1,1,1}, {0,0,1,1,1,0,0,0},
//		{0,0,1,1,1,0,0,1}
//};

#endif
