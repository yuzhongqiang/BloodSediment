/*
	Motor.c
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "detector.h"

#define MAX_TUBES 4

struct tube {
  u8 inplace;   // 0 - 在位; 1 - 不在位
  u8 remain_times;  // Remain times to move
  u8 values[MAX_MEASURE_TIMES];
};

struct tube tubes[MAX_TUBES];

/*
  MOTOR_ENx:  PA4	- 低电平有效
  MOTOR_DIRx: PA5
  MOTOR_CLKx: PA6 - 最快 100us
*/
#define MOTOR_ENX  PAout(4)	// PA4
#define MOTOR_DIRX PAout(5)	// PA5
#define MOTOR_CLKX PAout(6)	// PA6

/* 血沉值IO */
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
  步进电机的行程, 需要在联调时确定
*/
#define MOTOR_MOVEMENT 500
static u32 position = 0;     // 马达当前位置

 /* 读取10路血沉值 */
static void read_channels(void)
{

}

//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{ 		    		  			    
	if (TIM3->SR & 0X0001)  //溢出中断
	{
		position++;
		read_channels();

		MOTOR_CLKX = !MOTOR_CLKX;			    				   				     	    	
	}				   
	TIM3->SR &= ~(1 << 0);	//清除中断标志位 	    
}

/*
	定时器的计数频率 = 72,000,000/freq_div
	计数值 = count
*/
static void timer_init(u16 freq_div, u16 arr)
{
	/*
	通用定时器3初始化，时钟选择为APB2(72MHz)
		arr：		自动重装值
		freq_div：	时钟预分频数	
	*/
	RCC->APB1ENR |= (1 << 1);	// TIMER3时钟使能    
 	TIM3->ARR = arr;  			// 设定计数器自动重装值//刚好1ms    
	TIM3->PSC = freq_div - 1;	// 预分频器7200,得到10Khz的计数时钟

	/* 血沉值读数初始化 */

	TIM3->DIER |= (1 << 0);   	//允许更新中断				
	//TIM3->CR1 |= 0x01;      	//使能定时器3
  	nvic_init(1,3,TIM3_IRQChannel,2);//抢占1，子优先级3，组2
}

/*
  GPIOA在sys.c中已经使能
  配置PA4, PA5, PA6为通用推挽输出方式
*/
static void motor_init(void)
{
	/* ENx, DIRx, CLKx */
	GPIOA->CRL &= 0xF000FFFF;
	GPIOA->CRL |= 0xF333FFFF;

	/* 初始时输出高电平(?) */
	GPIOA->ODR |= (0x7 << 4); 

	/* 配置BLOOD_VALUEx为上拉/下拉输入模式 */
	GPIOA->CRH &= 0xFFFF0FF0;
	GPIOA->CRH |= 0x00008008;
	GPIOB->CRH &= 0x0000FFFF;
	GPIOB->CRH |= 0x88880000;
	
	GPIOC->CRL &= 0x00FFF000;
	GPIOC->CRL |= 0x88000333;
	GPIOC->CRH &= 0xFFFFFF00;
	GPIOC->CRH |= 0x00000088;

	/* MT_1, MT_2, MT_3初始输出高电平(?) */
	GPIOC->ODR |= 0x7;
}
 
void detector_init(void)
{
	u8 i = 0, j = 0;

	for (i=0; i<MAX_TUBES; i++)
	{
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
		tubes[i].remain_times = MAX_MEASURE_TIMES;
		for	(j=0; j<MAX_MEASURE_TIMES; j++)
			tubes[i].values[j] = 0;
	}

	motor_init();

	//Counter frequence is 10KHz, preload value is 500, total 50ms
	timer_init(7200, 500);
}

void start_detect(void)
{		
	GPIOA->ODR &= ~(1 << 4);	//ENx置低，使能电机
	TIM3->CR1 |= 0x01;      	//使能定时器3
}

void stop_detect(void)
{
	TIM3->CR1 &= ~(1 << 0);;    //使能定时器3
 	GPIOA->ODR |= (1 << 4);		//ENx置低，使能电机
}

