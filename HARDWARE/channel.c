/*
	detector.c
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "channel.h"
#include "delay.h"
#include "rtc.h"

struct tube tubes[MAX_CHANNELS];
u8 g_cur_chn = 0xff;   // 编号从0开始，当前正在检测的试管，0xff表示没有需要检查的试管
extern u8 g_scan_stage;
u8 g_need_reset;  // 一趟扫描结束，指示需要回复复位状态

void channel_close(void)
{
 	GPIOB->ODR |= 0x3;
	GPIOC->ODR |= (0x3 << 4);
	delay_ms(1);
}

void channel_open(u8 chn)
{
	switch (chn)
	{
	 case 0:
		GPIOB->ODR &= 0xfffc;
		GPIOC->ODR &= 0xffcf;
		break;
	 case 1:
		GPIOB->ODR &= 0xfffc;
		GPIOC->ODR &= 0xffcf;
		GPIOC->ODR |= (0x2 << 4);
		break;
	 case 2:
		GPIOB->ODR &= 0xfffc;
		GPIOC->ODR &= 0xffcf;
		GPIOC->ODR |= (0x1 << 4);
		break;
	 case 3:
		GPIOB->ODR &= 0xfffc;
		GPIOC->ODR &= 0xffcf;
		GPIOC->ODR |= (0x3 << 4);
		break;
#if !defined(SMALL_MACHINE)
	case 4:
		GPIOB->ODR &= 0xfffc;
		GPIOC->ODR &= 0xffcf;
		GPIOB->ODR |= 0x3;
		GPIOC->ODR |= (0x3 << 4);
#endif
	default:
		break;
	} 	
	delay_ms(5);	
}


/*
  MOTOR_ENx:  PA4	- 低电平有效
  MOTOR_DIRx: PA5
  MOTOR_CLKx: PA6 - 最快 100us
*/
#define MOTOR_ENX  PAout(6)	// PA4
#define MOTOR_DIRX PAout(5)	// PA5
#define MOTOR_CLKX PAout(4)	// PA6

/* 血沉值IO */
#define BLOOD_VALUE0  PBin(12)
#define BLOOD_VALUE1  PBin(13)
#define BLOOD_VALUE2  PBin(14)
#define BLOOD_VALUE3  PBin(15)
#define BLOOD_VALUE4  PCin(6)
#define BLOOD_VALUE5  PCin(7)
#define BLOOD_VALUE6  PCin(8)
#define BLOOD_VALUE7  PCin(9)
#define BLOOD_VALUE8  PAin(8)
#define BLOOD_VALUE9 PAin(11)

/*
  步进电机的行程, 需要在联调时确定
*/
#define MOTOR_MOVEMENT 500
//static u32 position = 0;     // 马达当前位置

/* 读取10路血沉值,返回值0、1 */
u8 channel_is_opaque(u8 chn)
{
	switch (chn)
	{
	 case 0:
	 	return !BLOOD_VALUE0;
	 case 1:
	 	return !BLOOD_VALUE1;
	 case 2:
	 	return !BLOOD_VALUE2;
	 case 3:
	 	return !BLOOD_VALUE3;
#ifndef SMALL_MACHINE

#endif
	default:
		return 1;
	}
}

/*
  GPIOA在sys.c中已经使能
  配置PA4, PA5, PA6为通用推挽输出方式
*/
static void _channel_config(void)
{
	/* 配置INA(PC4),INB(PC5),INC(PB0),IND(PB1)为输出模式 */
	/* 配置PC7为输入模式 */
	GPIOC->CRL &= 0x0F00FFFF;
	GPIOC->CRL |= 0x80330000;
	GPIOB->CRL &= 0xFFFFFF00;
	GPIOB->CRL |= 0x00000033;

	/* 默认输出到0xf通道(空) */
	GPIOB->ODR |= 0x3;
	GPIOC->ODR |= (0x3 << 4);
									
	/* 配置BLOIOD_VALUEx为上拉/下拉输入模式 */
	GPIOA->CRH &= 0xFFFF0FF0;
	GPIOA->CRH |= 0x00008008;
	GPIOB->CRH &= 0x0000FFFF;
	GPIOB->CRH |= 0x88880000;
	
	GPIOC->CRL &= 0x00FFF000;
	GPIOC->CRL |= 0x88000333;
	GPIOC->CRH &= 0xFFFFFF00;
	GPIOC->CRH |= 0x00000088;
}
 
void channel_init(void)
{
	u8 i = 0, j = 0;

	_channel_config();
	
	//初始化血沉值
	for (i=0; i<MAX_CHANNELS; i++)
	{
		channel_open(i);  
		switch (i)
		{
		case 0:
			tubes[i].inplace = !BLOOD_VALUE0;
			break;
		case 1:
			tubes[i].inplace = !BLOOD_VALUE1;
			break;
#ifndef SMALL_MACHINE
		case 2:
			tubes[i].inplace = !BLOOD_VALUE2;
			break;
		case 3:
			tubes[i].inplace = !BLOOD_VALUE3;
			break;

	  	case 4:
		case 5:
		case 6:
		case 7:
		case 8:	
		case 9:
			break;
#endif
		}
		channel_close();
				
		if (tubes[i].inplace && g_cur_chn == 0xff)
			g_cur_chn = i;
            if (tubes[i].inplace)
            {
                tubes[i].status = CHN_STATUS_WAITING;
			tubes[i].insert_time = rtc_get_sec();
            }
            else
                tubes[i].status = CHN_STATUS_NONE;
            
		tubes[i].remains = MAX_MEASURE_TIMES;
		for	(j=0; j<MAX_MEASURE_TIMES; j++)
			tubes[i].values[j] = 0;
	}
}

void channel_scan_all(void)
{
    u8 i, j;
    
	for (i=0; i<MAX_CHANNELS; i++)
	{
	    if  (tubes[i].status != CHN_STATUS_NONE)
            continue;
        
		channel_open(i);
		switch (i)
		{
		case 0:
			tubes[i].inplace = !BLOOD_VALUE0;
		case 1:			break;

			tubes[i].inplace = !BLOOD_VALUE1;
			break;
#ifndef SMALL_MACHINE
		case 2:
			tubes[i].inplace = !BLOOD_VALUE2;
			break;
		case 3:
			tubes[i].inplace = !BLOOD_VALUE3;
			break;

	  	case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			break;
#endif
		}			
		channel_close();
        
            if (tubes[i].inplace)
            {
                tubes[i].status = CHN_STATUS_WAITING;
			tubes[i].insert_time = rtc_get_sec();
            }
		else
                tubes[i].status = CHN_STATUS_NONE;
            
		tubes[i].remains = MAX_MEASURE_TIMES;
		for	(j=0; j<MAX_MEASURE_TIMES; j++)
			tubes[i].values[j] = 0;
	}
}

void channel_select_current(void)
{
	u8 i;

	g_cur_chn = 0xff;
	for (i=0; i<MAX_CHANNELS; i++)
	{
		if (!tubes[i].inplace)
			continue;
		else
		{
			if (((tubes[i].remains != 0) && (rtc_get_sec()-tubes[i].last_time > MOTOR0_INTERVAL_TIME)) ||
				(tubes[i].remains == 13))
			{
				g_cur_chn = i;
				return;
			}
		}
	}
}

void channel_scanning(void)
{
	tubes[g_cur_chn].status = CHN_STATUS_SCANNING;
	channel_open(g_cur_chn);
	motor_scan_chn(0, g_cur_chn);
}

void channel_main()
{
	channel_scan_all();
	
	if (g_scan_stage == SCAN_STAGE_SCANFINISH)
		channel_select_current();		

	if (g_scan_stage 
		channel_scanning();
		
}

