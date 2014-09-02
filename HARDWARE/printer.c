/*
* Printer
*
* printer.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "printer.h"

/*
	USART3中断服务程序
	注意,读取USARTx->SR能避免莫名其妙的错误   	
*/

/* 打印机使用USART3 */
void printer_init(u32 baud)
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
	USART3->CR1 |= 0X200C;  // 1位停止,无校验位.
}

u8 print_ch(u8 ch)
{
	while ((USART3->SR & 0x40) == 0)	//等待总线空闲
		;

	USART3->DR = ch;      
	return ch;
}

/* str必须以\0字符结尾 */
void print_str(u8* str)
{
 	while (*str++)
		print_ch(*str);
}
