/*
	Motor.c
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "motor.h"

/*
  MOTOR_ENx:  PA4	- �͵�ƽ��Ч
  MOTOR_DIRx: PA5
  MOTOR_CLKx: PA6 - ��� 100us
*/
#define MOTOR_ENX  PAout(4)	// PA4
#define MOTOR_DIRX PAout(5)	// PA5
#define MOTOR_CLKX PAout(6)	// PA6

/* Ѫ��ֵIO */
#define BLOOD_VALUE1  PBout(12)
#define BLOOD_VALUE2  PBout(13)
#define BLOOD_VALUE3  PBout(14)
#define BLOOD_VALUE4  PBout(15)
#define BLOOD_VALUE5  PCout(6)
#define BLOOD_VALUE6  PCout(7)
#define BLOOD_VALUE7  PCout(8)
#define BLOOD_VALUE8  PCout(9)
#define BLOOD_VALUE9  PAout(8)
#define BLOOD_VALUE10 PAout(11)

/*
  ����������г�, ��Ҫ������ʱȷ��
*/
#define MOTOR_MOVEMENT 500
u32 position = 0;     // ��ﵱǰλ��

 /* ��ȡ10·Ѫ��ֵ */
void read_channels(void)
{

}

//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if (TIM3->SR & 0X0001)  //����ж�
	{
		position++;
		read_channels();

		MOTOR_CLKX = !MOTOR_CLKX;			    				   				     	    	
	}				   
	TIM3->SR &= ~(1 << 0);	//����жϱ�־λ 	    
}

/*
	��ʱ���ļ���Ƶ�� = 72,000,000/freq_div
	����ֵ = count
*/
void timer_init(u16 freq_div, u16 arr)
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
	TIM3->CR1 |= 0x01;      	//ʹ�ܶ�ʱ��3
  	nvic_init(1,3,TIM3_IRQChannel,2);//��ռ1�������ȼ�3����2
}

/*
  GPIOA��sys.c���Ѿ�ʹ��
  ����PA4, PA5, PA6Ϊͨ�����������ʽ
*/
void motor_init(void)
{
	/* ENx, DIRx, CLKx */
	GPIOA->CRL &= 0xF000FFFF;
	GPIOA->CRL |= 0xF333FFFF;

	/* ��ʼʱ����ߵ�ƽ(?) */
	GPIOA->ODR |= (0x7 << 4); 

	/* ����BLOOD_VALUExΪ����/��������ģʽ */
	GPIOA->CRH &= 0xFFFF0FF0;
	GPIOA->CRH |= 0x00008008;
	GPIOB->CRH &= 0x0000FFFF;
	GPIOB->CRH |= 0x88880000;
	
	GPIOC->CRL &= 0x00FFF000;
	GPIOC->CRL |= 0x88000333;
	GPIOC->CRH &= 0xFFFFFF00;
	GPIOC->CRH |= 0x00000088;

	/* MT_1, MT_2, MT_3��ʼ����ߵ�ƽ(?) */
	GPIOC->ODR |= 0x7;
}
