/*
* Communicate with PC
*
* comm.c
*
*/

#include <stm32f10x_lib.h>	
#include "sys.h"
#include "comm.h"	 

/* PC communication uses USART4 */
void comm_init(u32 baud)
{  	 
#if 0
	float temp;
	u16 mantissa, fraction;
		   
	RCC->APB2ENR |= 1 << 4;   	//使能PORTC口时钟  OK
	RCC->APB1ENR |= 1 << 19;  	//使能串口时钟 		OK
	GPIOC->CRH &= 0XFFFF00FF; 
	GPIOC->CRH |= 0X00008B00;		//IO状态设置	 OK
		  
	RCC->APB1RSTR |= (1 << 19);   	//复位串口4		OK
	RCC->APB1RSTR &= ~(1 << 19);	//停止复位	   	OK 

	//波特率设置
	temp = (float)(PCLK1_FREQ / (baud * 16));	//得到USARTDIV
	mantissa = temp;						 	//得到整数部分
	fraction = (temp - mantissa) * 16; 			//得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART4->BRR = mantissa; // 波特率设置	 
	USART4->CR1 |= 0X200C;  // 1位停止,无校验位.

#ifdef EN_USART4_RX		  	//如果使能了接收
	//使能接收中断
	USART4->CR1 |= (1 << 8);    //PE中断使能
	USART4->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	    	
	nvic_init(3, 3, USART4_IRQChannel, 2);//组2，最低优先级 
#endif
#endif
}

