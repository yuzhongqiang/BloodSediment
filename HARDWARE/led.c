#include <stm32f10x_lib.h>	   
#include "led.h"
#include "delay.h"	   

//初始化PA8和PD2为输出口.并使能这两个口的时钟		    
//LED IO初始化
void led_init(void)
{
	RCC->APB2ENR|=1<<2;    //使能PORTA时钟	   	 
	RCC->APB2ENR|=1<<5;    //使能PORTD时钟	
	   	 
	GPIOA->CRH&=0XFFFFFFF0; 
	GPIOA->CRH|=0X00000003;//PA8 推挽输出   	 
    GPIOA->ODR|=1<<8;      //PA8 输出高
											  
	GPIOD->CRL&=0XFFFFF0FF;
	GPIOD->CRL|=0X00000300;//PD.2推挽输出
	GPIOD->ODR|=1<<2;      //PD.2输出高 
}

//跑马灯测试程序
void led_run(void)
{
 	while(1)
	{
		LED0=0;
		LED1=1;
		delay_ms(300);
		LED0=1;
		LED1=0;
		delay_ms(300);
	}	
}

