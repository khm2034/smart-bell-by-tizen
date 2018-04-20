#ifndef __SMARTBELL_MOTOR_H__
#define __SMARTBELL_MOTOR_H__

int start_dc_motor(int motor_id, int speed);
int stop_dc_motor(int motor_id);
int narrow_arm();
int extend_arm();
int stop_arm();
#endif
