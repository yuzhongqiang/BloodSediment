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

#define SCAN_STAGE_RESETING 1
#define SCAN_STAGE_RESETED  2
#define SCAN_STAGE_SCANNING 3
#define SCAN_STAGE_SCANFINISH 4

u8 channel_is_opaque(u8 chn);
void channel_init(void);
void channel_main(void);
void channel_pause(void);
void channel_resume(void);


#endif

