#ifndef __MOTOR_H
#define __MOTOR_H

/*
* ÿ��ͨ����״̬
*/
#define CHN_STATUS_NONE    0
#define CHN_STATUS_WAITING 1
#define CHN_STATUS_SUSPEND 2
#define CHN_STATUS_SCANNING 3
#define CHN_STATUS_FINISH    4
#define CHN_STATUS_ERROR  5

struct tube {
  u8 inplace;   		 // 0 - ��λ; 1 - ����λ
  u8 status;                // 0 - ������1 - ���� 2 - ��������
  u8 pre_status;         // ǰ��ɨ���״̬
  u32 insert_time;      //��һ�μ�⵽�����ʱ��
  u32 last_scan_time;        // �ϴμ�������ʱ��
  u8 remains;  	      // Remain times to move

  u32 scan_times[MAX_MEASURE_TIMES];      //��ʼĳ��ɨ���ʱ��
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

