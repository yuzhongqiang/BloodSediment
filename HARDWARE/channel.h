#ifndef __MOTOR_H
#define __MOTOR_H



/*
* 每个通道的状态
*/
#define CHN_STATUS_NONE    0
#define CHN_STATUS_WAITING 1
#define CHN_STATUS_SUSPEND 2
#define CHN_STATUS_SCANNING 3
#define CHN_STATUS_FINISH    4
#define CHN_STATUS_ERROR  5

struct tube {
  u8 inplace;   		 // 0 - 在位; 1 - 不在位
  u8 status;                // 0 - 结束；1 - 挂起； 2 - 正在运行
  u8 pre_status;         // 前次扫描的状态
  u32 insert_time;      //第一次检测到插入的时间
  u32 last_scan_time;        // 上次检查结束的时间
  u8 remains;  	      // Remain times to move

  u32 scan_times[MAX_MEASURE_TIMES];      //开始某次扫描的时间
  u32 values[MAX_MEASURE_TIMES];
};





void detector_init(void);
void start_detect(void);
void stop_detect(void);
u8 detect_finished(void);
void channel_scanning(void);
u8 position_reset(void);  
void set_direction(u8 dir);
 void motor_drive(u8 dir,u32 stp);

void start_chn(u8);
void stop_chn(void);
u8 motor_reset(void);

u8 is_motor_reset(void);
u8 read_channel(u8 chn);
void select_channel(void);
void scan_channels(void);

#endif
