#ifndef __MOTOR_H_
#define __MOTOR_H_

#define MOTOR0_MAX_TRIP  3200
#define MOTOR0_INTERVAL_TIME 10

#define SCAN_STAGE_INIT  0
#define SCAN_STAGE_RESETING 1
#define SCAN_STAGE_RESETED  2
#define SCAN_STAGE_SCANNING 3
#define SCAN_STAGE_SCANFINISH 4


void detector_init(void);
void start_detect(void);
void stop_detect(void);
u8 detect_finished(void);
void channel_scanning(void);
u8 position_reset(void);  
void set_direction(u8 dir);
 void motor_drive(u8 dir,u32 stp);
u8 motor_reset(void);
u8 is_motor_reset(void);

#endif     /* __MOTOR_H_ */

