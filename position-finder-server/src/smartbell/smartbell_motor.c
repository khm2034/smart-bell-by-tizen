#include "smartbell/smartbell_motor.h"
#include "resource.h"
#include "log.h"
int start_dc_motor(int motor_id, int speed){
	int ret;
	ret = resource_set_dc_motor_driver_L298N_speed(motor_id, speed);
	return ret;
}

int stop_dc_motor(int motor_id){
	int ret;
	ret = resource_set_dc_motor_driver_L298N_speed(motor_id, 0);
	return ret;
}
