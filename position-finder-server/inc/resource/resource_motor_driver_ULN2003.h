#ifndef __RESOURCE_MOTOR_DRIVER_ULN2003_H__
#define __RESOURCE_MOTOR_DRIVER_ULN2003_H__

#define DEFAULT_STEP_MOTOR1_PIN1 18
#define DEFAULT_STEP_MOTOR1_PIN2 24
#define DEFAULT_STEP_MOTOR1_PIN3 25
#define DEFAULT_STEP_MOTOR1_PIN4 5

#define DEFAULT_STEP_MOTOR2_PIN1 4
#define DEFAULT_STEP_MOTOR2_PIN2 17
#define DEFAULT_STEP_MOTOR2_PIN3 27
#define DEFAULT_STEP_MOTOR2_PIN4 22


typedef enum {
	ULN2003_MOTOR_ID_1,
	ULN2003_MOTOR_ID_2,
	ULN2003_MOTOR_ID_MAX
} ULN2003_step_motor_id_e;

typedef enum {
	ULN2003_MOTOR_STEP_1 = 1,
	ULN2003_MOTOR_STEP_2,
	ULN2003_MOTOR_STEP_3,
	ULN2003_MOTOR_STEP_4,
	ULN2003_MOTOR_STEP_5,
	ULN2003_MOTOR_STEP_6,
	ULN2003_MOTOR_STEP_7,
	ULN2003_MOTOR_STEP_8,
}ULN2003_motor_step;

int resource_set_motor_driver_ULN2003_speed(ULN2003_step_motor_id_e id, int delay, int dir);

int resource_set_motor_driver_ULN2003_configuration(ULN2003_step_motor_id_e id,
		unsigned int pin1, unsigned int pin2, unsigned int pin3, unsigned int pin4);

#endif
