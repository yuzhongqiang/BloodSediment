/*
* Card reader
*
* reader.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "reader.h"

/* IC Reader uses USART2 */
void reader_init(u32 baud)
{
	float temp;
	u16 mantissa, fraction;
		   
	RCC->APB2ENR |= 1 << 2;   //使能PORTA口时钟  OK
	RCC->APB1ENR |= 1 << 17;  	//使能串口2时钟 
	GPIOA->CRL &= 0XFFFF00FF; 
	GPIOA->CRL |= 0X00008B00;	//IO状态设置	OK
		  
	RCC->APB1RSTR |= (1 << 17); //复位串口2	   OK
	RCC->APB1RSTR &= ~(1<<17);	//停止复位	   OK  

	//波特率设置
	temp = (float)(PCLK1_FREQ / (baud * 16));	//得到USARTDIV
	mantissa = temp;						 	//得到整数部分
	fraction = (temp - mantissa) * 16; 			//得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART2->BRR = mantissa; // 波特率设置	 
	USART2->CR1 |= 0X200C;  //1位停止,无校验位.

#ifdef EN_USART2_RX		  //如果使能了接收
	//使能接收中断
	USART2->CR1 |= (1 << 8);    //PE中断使能
	USART2->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	    	
	nvic_init(3, 3, USART2_IRQChannel, 2);//组2，最低优先级 
#endif
}
