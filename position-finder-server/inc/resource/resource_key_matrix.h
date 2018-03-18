#ifndef __POSITION_FINDER_RESOURCE_KEY_MATRIX_H__
#define __POSITION_FINDER_RESOURCE_KEY_MATRIX_H__

/* Default GPIO pins of raspberry pi 3 connected with ROW pins of key matrix */
#define DEFAULT_KEY_MATRIX_ROW1 21
#define DEFAULT_KEY_MATRIX_ROW2 20
#define DEFAULT_KEY_MATRIX_ROW3 16
#define DEFAULT_KEY_MATRIX_ROW4 12

/* Default GPIO pins of raspberry pi 3 connected with COL pins of key matrix */
#define DEFAULT_KEY_MATRIX_COL1 26
#define DEFAULT_KEY_MATRIX_COL2 19
#define DEFAULT_KEY_MATRIX_COL3 13
#define DEFAULT_KEY_MATRIX_COL4 6

static char key_matrix_value[4][4] = {{'1', '2', '3', 'A'},
									{'4', '5', '6', 'B'},
									{'7', '8', '9', 'C'},
									{'*', '0', '#', 'D'}};
int resource_read_key_matrix(char* getch);
void resource_close_key_matrix();

#endif
