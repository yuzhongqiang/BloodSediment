#ifndef __MOTOR_H_
#define __MOTOR_H_

#define MOTOR0_MIN_STEP  2     /* motor0ת��һȦ�ƶ��ľ���,��λ0.01mm */
#define MOTOR0_MAX_TRIP  2200
#define MOTOR0_INTERVAL_TIME 10

void motor_move_steps(u8 motor_id, u8 dir, u32 steps);
void motor_move_steps_blocked(u8 motor_id, u8 dir, u32 steps);
void motor_reset_position_blocked(u8 motor_id);
void motor_reset_position(u8 motor_id);
void motor_scan_chn(u8 motor_id);
void motor_init(void);
void motor2_shake(u32 steps);
void _motor_check_shake(void);

#endif     /* __MOTOR_H_ */

