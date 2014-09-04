/*
* rtc.h
*/
#ifndef __RTC_H
#define __RTC_H	    

//时间结构体
typedef struct 
{
	u8 hour;
	u8 min;
	u8 sec;			
	//公历日月年周
	u16 w_year;
	u8  w_month;
	u8  w_date;
	u8  week;		 
}tm;					 
extern tm timer; 

extern u8 const mon_table[12];//月份日期数据表
void disp_time(u8 x,u8 y,u8 size);//在制定位置开始显示时间
void disp_week(u8 x,u8 y,u8 size,u8 lang);//在指定位置显示星期
u8 rtc_init(void);        //初始化RTC,返回0,失败;1,成功;
u8 is_leap_year(u16 year);//平年,闰年判断
u8 rtc_get(void);         //更新时间   
u8 rtc_get_week(u16 year,u8 month,u8 day);
u8 rtc_set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//设置时间	  
//void auto_time_set(void);//设置时间为编译时间
u32 rtc_get_sec(void);  // 获取系统运行的秒钟数

#endif




























 
