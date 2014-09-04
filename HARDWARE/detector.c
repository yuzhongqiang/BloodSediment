/*
	Motor.c
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "detector.h"
#include "delay.h"
#include "rtc.h"

#define MAX_TUBES 4

u8 g_busy = 0;
u32 g_stp;

/* ���ֹͣ�������е�ԭ��
   0 - ���о���ﵽ
   1 - ������е���λλ��
   2 - ��⵽Ѫ��ֵ�ı�
*/
#define STOP_REASON_ARRIVED 0
#define STOP_REASON_RESET	1
#define STOP_REASON_VALUE_CHG 2
u8  g_stop_reason = 0;

#define MOTOR_MAX_TRIP  10000
u32 g_trip;
u8  g_tripreset = 0;


struct tube {
  u8 inplace;   		// 0 - ��λ; 1 - ����λ
  u8 status;            // 0 - ������1 - ���� 2 - ��������
  u32 last_time;        // �ϴμ�������ʱ��
  u8 remain_times;  	// Remain times to move
  u8 values[MAX_MEASURE_TIMES];
};

struct tube tubes[MAX_TUBES];
u8 g_cur_chn = 0xff;   // ��Ŵ�0��ʼ����ǰ���ڼ����Թܣ�0xff��ʾû����Ҫ�����Թ�
u32 g_cur_trip;   //Current trip to change value

void stop_chn(void)
{
 	GPIOB->ODR |= 0x3;
	GPIOC->ODR |= (0x3 << 4);
}

void start_chn(u8 chn)
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
}


/*
  MOTOR_ENx:  PA4	- �͵�ƽ��Ч
  MOTOR_DIRx: PA5
  MOTOR_CLKx: PA6 - ��� 100us
*/
#define MOTOR_ENX  PAout(6)	// PA4
#define MOTOR_DIRX PAout(5)	// PA5
#define MOTOR_CLKX PAout(4)	// PA6

/* Ѫ��ֵIO */
#define BLOOD_VALUE0  PBout(12)
#define BLOOD_VALUE1  PBout(13)
#define BLOOD_VALUE2  PBout(14)
#define BLOOD_VALUE3  PBout(15)
#define BLOOD_VALUE4  PCout(6)
#define BLOOD_VALUE5  PCout(7)
#define BLOOD_VALUE6  PCout(8)
#define BLOOD_VALUE7  PCout(9)
#define BLOOD_VALUE8  PAout(8)
#define BLOOD_VALUE9 PAout(11)

/*
  ����������г�, ��Ҫ������ʱȷ��
*/
#define MOTOR_MOVEMENT 500
//static u32 position = 0;     // ��ﵱǰλ��

/* ��ȡ10·Ѫ��ֵ,����ֵ0��1 */
u8 read_channel(u8 chn)
{
	switch (chn)
	{
	 case 0:
	 	return BLOOD_VALUE0;
	 case 1:
	 	return BLOOD_VALUE1;
	 case 2:
	 	return BLOOD_VALUE2;
	 case 3:
	 	return BLOOD_VALUE3;
#ifndef SMALL_MACHINE

#endif
	default:
		return 1;
	}
}

/*
	ϵͳ�ϵ�ʱ�������λλ��
	�����ڵ����ʼ�����ִ��	
*/
void reset_motor(void)
{
	while (motor_reset())
	{
	 	motor_drive(1, 250);
	}

	while (!motor_reset())
	{
		 motor_drive(0, 50);
		 //delay_ms(500);
	}
}

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 	
	u16 temp = 0;
//	u8 is_reset;
		    		  			    
	if (TIM3->SR & 0X0001)  //����ж�
	{
		temp = GPIOA->ODR;
		temp ^= 0x0010;
		GPIOA->ODR = temp;

		switch (g_stop_reason)
		{
		case STOP_REASON_ARRIVED:  // Trip arrived
			g_stp--;
			if(g_stp==0)
				g_busy=0;
			break;
		case STOP_REASON_RESET:	 // Reset position arrived
			g_busy = motor_reset()? 0 : 1;
			break;
		case STOP_REASON_VALUE_CHG:  // Blood value detected
			g_cur_trip++;
			tubes[g_cur_chn].values[13 - tubes[g_cur_chn].remain_times] = g_cur_trip;
			g_busy = read_channel(g_cur_chn)? 0 : 1;
			if (!g_busy)
				tubes[g_cur_chn].remain_times--;
			if (!tubes[g_cur_chn].remain_times)
				tubes[g_cur_chn].last_time = rtc_get_sec();
			break;
		}			   				     	    	
	}				   
	TIM3->SR &= ~(1 << 0);	//����жϱ�־λ 	    
}

/*
	��ʱ���ļ���Ƶ�� = 72,000,000/freq_div
	����ֵ = count
*/
#if 1
static void timer_init(u16 freq_div, u16 arr)
{
	/*
	ͨ�ö�ʱ��3��ʼ����ʱ��ѡ��ΪAPB2(72MHz)
		arr��		�Զ���װֵ
		freq_div��	ʱ��Ԥ��Ƶ��	
	*/
	RCC->APB1ENR |= (1 << 1);	// TIMER3ʱ��ʹ��    
 	TIM3->ARR = arr;  			// �趨�������Զ���װֵ//�պ�1ms    
	TIM3->PSC = freq_div - 1;	// Ԥ��Ƶ��7200,�õ�10Khz�ļ���ʱ��

	/* Ѫ��ֵ������ʼ�� */

	TIM3->DIER |= (1 << 0);   	//��������ж�				
	//TIM3->CR1 |= 0x01;      	//ʹ�ܶ�ʱ��3
  	nvic_init(1,3,TIM3_IRQChannel,2);//��ռ1�������ȼ�3����2
}
#endif

/*
  GPIOA��sys.c���Ѿ�ʹ��
  ����PA4, PA5, PA6Ϊͨ�����������ʽ
*/
static void motor_init(void)
{
	/* ENx(PA6), DIRx(PA5), CLKx(PA4)�����ģʽ */
	GPIOA->CRL &= 0xF000FFFF;
	GPIOA->CRL |= 0x03330000;
	//GPIOA->CRL |= 0x0BBB0000;

	/* ��ʼʱ����͵�ƽ��Enx low active */
	GPIOA->ODR |= 0x0040;
			
	/* ����INA(PC4),INB(PC5),INC(PB0),IND(PB1)Ϊ���ģʽ */
	/* ����PC7Ϊ����ģʽ */
	GPIOC->CRL &= 0x0F00FFFF;
	GPIOC->CRL |= 0x80330000;
	GPIOB->CRL &= 0xFFFFFF00;
	GPIOB->CRL |= 0x00000033;

	/* Ĭ�������0xfͨ��(ʵ��Ϊ��) */
	GPIOB->ODR |= 0x3;
	GPIOC->ODR |= (0x3 << 4);
									
	/* ����BLOIOD_VALUExΪ����/��������ģʽ */
	GPIOA->CRH &= 0xFFFF0FF0;
	GPIOA->CRH |= 0x00008008;
	GPIOB->CRH &= 0x0000FFFF;
	GPIOB->CRH |= 0x88880000;
	
	GPIOC->CRL &= 0x00FFF000;
	GPIOC->CRL |= 0x88000333;
	GPIOC->CRH &= 0xFFFFFF00;
	GPIOC->CRH |= 0x00000088;

	/* MT_1(PC0), MT_2(PC1), MT_3(PC2)��ʼ����ߵ�ƽ */
	GPIOC->ODR |= 0x1;    //0x7;

	// ����ظ�����ʼλ��
	reset_motor();
}
 
void detector_init(void)
{
	u8 i = 0, j = 0;

	//Counter frequence is 10KHz, preload value is 500, total 50ms
	timer_init(7200, 1);

	motor_init();

	//��ʼ��Ѫ��ֵ
	for (i=0; i<MAX_TUBES; i++)
	{
		start_chn(i);
		switch (i)
		{
		case 0:
			tubes[i].inplace = BLOOD_VALUE0;
			break;
		case 1:
			tubes[i].inplace = BLOOD_VALUE1;
			break;
		case 2:
			tubes[i].inplace = BLOOD_VALUE2;
			break;
		case 3:
			tubes[i].inplace = BLOOD_VALUE3;
			break;
#ifndef SMALL_MACHINE
	  	case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			break;
#endif
		}
		if (tubes[i].inplace && g_cur_chn == 0xff)
			g_cur_chn = i;

		stop_chn();
		tubes[i].remain_times = MAX_MEASURE_TIMES;
		for	(j=0; j<MAX_MEASURE_TIMES; j++)
			tubes[i].values[j] = 0;
	}
}

void start_motor(void)
{							
	GPIOA->ODR &= 0xffBf;
	TIM3->CR1 |= 0x01;      	//ʹ�ܶ�ʱ��3
}

void stop_detect(void)
{
	TIM3->CR1 &= ~(1 << 0);;    //�رն�ʱ��3
 	GPIOA->ODR |= 0x0040;	//ENx�øߣ��رյ��
}

u8 detect_finished(void)
{
	u8 i;
 	
	for (i=0; i<MAX_TUBES; i++)
	{
		if (tubes[i].inplace && (tubes[i].remain_times > 0))
		{
			g_cur_chn = i;
			return 0;
		}
	}
	return 1;
}

void select_channel(void)
{
	u8 i;

	g_cur_trip = 0;
	g_cur_chn = 0xff;
	for (i=0; i<MAX_TUBES; i++)
	{
		if (!tubes[i].inplace)
			continue;
		else
		{
			if ((tubes[i].remain_times != 0) &&
				(rtc_get_sec()-tubes[i].last_time > 60))
			{
				g_cur_chn = i;
				return;
			}
		}
	}
}

void do_detect(void)
{
	if (g_cur_chn == 0xff)
		return;

	tubes[g_cur_chn].status = 2;  // Running
	g_stop_reason = STOP_REASON_VALUE_CHG;
	motor_drive(1, MOTOR_MAX_TRIP);
}

/* ����ֵ��
	1 - ������е����·�
	0 - ���û�����е����·�
*/
u8 motor_reset(void)
{
	u16 pcv = GPIOC->IDR;
  	return (pcv & 0x0080) ? 1 : 0;
}

 /* ���õ������
    0 - ����
	1 - ����
*/
void set_direction(u8 dir)
{
	if (!dir)
		GPIOA->ODR |= 0x0020;
	else
		GPIOA->ODR &= 0xffdf;	   
}

void motor_drive(u8 dir, u32 stp)
{
	set_direction(dir);
	g_busy = 1;
	g_stp = stp*2;
	start_motor();
	while (g_busy);
		stop_detect();			
}

