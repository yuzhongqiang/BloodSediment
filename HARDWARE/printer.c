/*
* Printer
*
* printer.c
*
*/

#include <stm32f10x_lib.h>
#include "printer.h"



/* 打印机使用USART3 */
void printer_init(void)
{
	float temp;
	u16 mantissa, fraction;
		   
	RCC->APB1ENR |= 1 << 18;  	//使能串口时钟  OK
	GPIOB->CRH &= 0XFFFF00FF; 
	GPIOB->CRH |= 0X00008B00;	//IO状态设置  OK
		  
	RCC->APB1RSTR |= (1 << 18);   //复位串口3	 OK
	RCC->APB1RSTR &= ~(1 << 18);//停止复位	   	  OK

	//波特率设置
	temp = (float)(PCLK1_FREQ / (baud * 16));	//得到USARTDIV
	mantissa = temp;						 	//得到整数部分
	fraction = (temp - mantissa) * 16; 			//得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART3->BRR = mantissa; // 波特率设置	 
	USART3->CR1 |= 0X200C;  //1位停止,无校验位.

#ifdef EN_USART3_RX		  //如果使能了接收
	//使能接收中断
	USART3->CR1 |= (1 << 8);    //PE中断使能
	USART3->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	    	
	nvic_init(3, 3, USART3_IRQChannel, 2);//组2，最低优先级 
#endif
}

