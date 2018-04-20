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
	resource_close_dc_motor_driver_L298N(motor_id);
	return ret;
}

int narrow_arm()
{
	int ret;
	ret = resource_set_motor_driver_ULN2003_speed(0, 0, 0); // 오른
	if(ret == -1)
		return -1;
	ret = resource_set_motor_driver_ULN2003_speed(1, 0, 1); //왼
	if (ret == -1)
		return -1;
	return 0;
}

int extend_arm()
{
	int ret;
	ret = resource_set_motor_driver_ULN2003_speed(0, 0, 1); // 오른
	if (ret == -1)
			return -1;
	ret = resource_set_motor_driver_ULN2003_speed(1, 0, 0); //왼
	if (ret == -1)
		return -1;
	return 0;
}

int stop_arm()
{
	int ret;
	ret = resource_set_motor_driver_ULN2003_speed(0, 0, -1); // 오른
	if(ret == -1)
		return -1;
	ret = resource_set_motor_driver_ULN2003_speed(1, 0, -1); //왼
	if (ret == -1)
		return -1;
	resource_close_motor_driver_ULN2003_all();
	return 0;
}
