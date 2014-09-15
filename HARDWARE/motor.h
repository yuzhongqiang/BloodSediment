#ifndef __MOTOR_H_
#define __MOTOR_H_

#define MOTOR0_MAX_TRIP  3200
#define MOTOR0_INTERVAL_TIME 10

void motor_move_steps(u8 motor_id, u8 dir, u32 steps);
void motor_move_steps_blocked(u8 motor_id, u8 dir, u32 steps);
void motor_reset_position_blocked(u8 motor_id);
void motor_reset_position(u8 motor_id);
void motor_scan_chn(u8 motor_id, u8 chn_id);
void motor_init(void);


#endif     /* __MOTOR_H_ */

