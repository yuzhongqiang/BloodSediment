/*
	Motor.c
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "detector.h"

#define MAX_TUBES 4

struct tube {
  u8 inplace;   		// 0 - ��λ; 1 - ����λ
  u8 remain_times;  	// Remain times to move
  u8 values[MAX_MEASURE_TIMES];
};

struct tube tubes[MAX_TUBES];
u8 current_tube = 0xff;   // ��ǰ���ڼ����Թܣ�0xff��ʾû����Ҫ�����Թ�

//u8 chn_data[2][10] = {
//  {0,0}, {}, {}, {}, {}, {}, {}, {}, {}, {}
//};

void stop_chns(void)
{
 	GPIOB->ODR |= 0x3;
	GPIOC->ODR |= (0x3 << 4);
}

void start_chn(u8 chn)
{
 	GPIOB->ODR &= 0xfffc;
	GPIOC->ODR &= 0xffcf;
	
}

void start_chn0(void)
{
	GPIOB->ODR &= 0xfffc;
	GPIOC->ODR &= 0xffcf;
}

void start_chn1(void)
{
	GPIOB->ODR &= 0xfffc;
	GPIOC->ODR &= 0xffcf;
	GPIOC->ODR |= (0x1 << 4);
}

void start_chn2(void)
{
	GPIOB->ODR &= 0xfffc;
	GPIOC->ODR &= 0xffcf;
	GPIOC->ODR |= (0x2 << 4);
}

void start_chn3(void)
{
	GPIOB->ODR &= 0xfffc;
	GPIOC->ODR &= 0xffcf;
	GPIOC->ODR |= (0x3 << 4);
}

#if !defined(SMALL_MACHINE)
void start_chn4(void)
{
	GPIOB->ODR &= 0xfffc;
	GPIOC->ODR &= 0xffcf;

	GPIOB->ODR |= 0x3;
	GPIOC->ODR |= (0x3 << 4);
}
#endif

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
static u32 position = 0;     // ��ﵱǰλ��

 /* ��ȡ10·Ѫ��ֵ */
static void read_channels(void)
{

}

u8 is_motor_reset(void)
{
	return 1;
}

void motor_step_down(void)
{
}

void motor_step_up(void)
{
}

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 	
	u16 temp;

	temp = GPIOA->ODR;
	temp ^= 0x0010;
	GPIOA->ODR = temp;
		    		  			    
	if (TIM3->SR & 0X0001)  //����ж�
	{
		//position++;
		//read_channels();

		//MOTOR_CLKX = !MOTOR_CLKX;			    				   				     	    	
	}				   
	TIM3->SR &= ~(1 << 0);	//����жϱ�־λ 	    
}

/*
	��ʱ���ļ���Ƶ�� = 72,000,000/freq_div
	����ֵ = count
*/
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

	/* ��ʼʱ����͵�ƽ(?)��Enx/DIRx High active */
	GPIOA->ODR |= (0x7 << 4); 
	//GPIOA->ODR &= 0xff8f;
			
	/* ����INA(PC4),INB(PC5),INC(PB0),IND(PB1)Ϊ���ģʽ */
	GPIOC->CRL &= 0xFF00FFFF;
	GPIOC->CRL |= 0x00330000;
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
	while (!is_motor_reset())
		 motor_step_down();
}
 
void detector_init(void)
{
	u8 i = 0, j = 0;

	for (i=0; i<MAX_TUBES; i++)
	{
		switch (i)
		{
		case 0:
			start_chn0();
			tubes[i].inplace = BLOOD_VALUE0;
			break;
		case 1:
			start_chn1();
			tubes[i].inplace = BLOOD_VALUE1;
			break;
		case 2:
			start_chn2();
			tubes[i].inplace = BLOOD_VALUE2;
			break;
		case 3:
			start_chn3();
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
		stop_chns();
		tubes[i].remain_times = MAX_MEASURE_TIMES;
		for	(j=0; j<MAX_MEASURE_TIMES; j++)
			tubes[i].values[j] = 0;
	}

	//Counter frequence is 10KHz, preload value is 500, total 50ms
	timer_init(7200, 50000);

	motor_init();
}

void start_detect(void)
{							
	GPIOA->ODR |= (0x6 << 4);
	//GPIOA->ODR &= 0xff9f;
	TIM3->CR1 |= 0x01;      	//ʹ�ܶ�ʱ��3
}

void stop_detect(void)
{
	TIM3->CR1 &= ~(1 << 0);;    //�رն�ʱ��3
 	GPIOA->ODR &= 0xff8f;	//ENx�õͣ��رյ��
}

u8 detect_finished(void)
{
	u8 i;
 	
	for (i=0; i<MAX_TUBES; i++)
	{
		if (tubes[i].inplace && (tubes[i].remain_times > 0))
		{
			current_tube = i;
			return 0;
		}
	}
	return 1;
}

void do_detect(void)
{

}
