/*
* rtc.h
*/
#ifndef __RTC_H
#define __RTC_H	    

//ʱ��ṹ��
typedef struct 
{
	u8 hour;
	u8 min;
	u8 sec;			
	//������������
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;		 
}tm;					 
extern tm timer; 

extern u8 const mon_table[12];//�·��������ݱ�
void disp_time(u8 x,u8 y,u8 size);//���ƶ�λ�ÿ�ʼ��ʾʱ��
void disp_week(u8 x,u8 y,u8 size,u8 lang);//��ָ��λ����ʾ����
u8 rtc_init(void);        //��ʼ��RTC,����0,ʧ��;1,�ɹ�;
u8 is_leap_year(u16 year);//ƽ��,�����ж�
u8 rtc_get(void);         //����ʱ��   
u8 rtc_get_week(u16 year,u8 month,u8 day);
u8 rtc_set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//����ʱ��	  
//void auto_time_set(void);//����ʱ��Ϊ����ʱ��
u32 rtc_get_sec(void);  // ��ȡϵͳ���е�������

#endif




























 
