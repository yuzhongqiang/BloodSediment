#ifndef __MOTOR_H
#define __MOTOR_H

/*
 
*/

void detector_init(void);
void start_detect(void);
void stop_detect(void);
u8 detect_finished(void);
void do_detect(void);
u8 position_reset(void);  
void set_direction(u8 dir);
 void motor_drive(u8 dir,u32 stp);

void start_chn(u8);
void stop_chn(void);
u8 motor_reset(void);

u8 is_motor_reset(void);
u8 read_channel(u8 chn);


#endif
