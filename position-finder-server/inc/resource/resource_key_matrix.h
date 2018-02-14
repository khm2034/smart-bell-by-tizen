#ifndef __POSITION_FINDER_RESOURCE_KEY_MATRIX_H__
#define __POSITION_FINDER_RESOURCE_KEY_MATRIX_H__

/* Default GPIO pins of raspberry pi 3 connected with ROW pins of key matrix */
#define DEFAULT_KEY_MATRIX_ROW1 26
#define DEFAULT_KEY_MATRIX_ROW2 20
#define DEFAULT_KEY_MATRIX_ROW3 19
#define DEFAULT_KEY_MATRIX_ROW4 16

/* Default GPIO pins of raspberry pi 3 connected with COL pins of key matrix */
#define DEFAULT_KEY_MATRIX_COL1 6
#define DEFAULT_KEY_MATRIX_COL2 12
#define DEFAULT_KEY_MATRIX_COL3 22
#define DEFAULT_KEY_MATRIX_COL4 23

static char key_matrix_value[4][4] = {{'1', '2', '3', 'A'},
									{'4', '5', '6', 'B'},
									{'7', '8', '9', 'C'},
									{'*', '0', '#', 'D'}};
extern int resource_read_key_matrix();

#endif /* __POSITION_FINDER_RESOURCE_LED_H__ */
