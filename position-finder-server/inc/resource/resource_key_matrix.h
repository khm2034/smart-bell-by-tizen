#ifndef __POSITION_FINDER_RESOURCE_KEY_MATRIX_H__
#define __POSITION_FINDER_RESOURCE_KEY_MATRIX_H__

/* Default GPIO pins of raspberry pi 3 connected with ROW pins of key matrix */
#define DEFAULT_KEY_MATRIX_ROW1 6
#define DEFAULT_KEY_MATRIX_ROW2 13
#define DEFAULT_KEY_MATRIX_ROW3 19
#define DEFAULT_KEY_MATRIX_ROW4 26
//#define DEFAULT_KEY_MATRIX_ROW1 4 //7
//#define DEFAULT_KEY_MATRIX_ROW2 18 //12
//#define DEFAULT_KEY_MATRIX_ROW3 5 //29
//#define DEFAULT_KEY_MATRIX_ROW4 6 //31

/* Default GPIO pins of raspberry pi 3 connected with COL pins of key matrix */
#define DEFAULT_KEY_MATRIX_COL1 12
#define DEFAULT_KEY_MATRIX_COL2 16
#define DEFAULT_KEY_MATRIX_COL3 20
#define DEFAULT_KEY_MATRIX_COL4 21

static char key_matrix_value[4][4] = {{'1', '2', '3', 'A'},
									{'4', '5', '6', 'B'},
									{'7', '8', '9', 'C'},
									{'*', '0', '#', 'D'}};
extern int resource_read_key_matrix();

#endif
