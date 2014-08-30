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

/* 串口2中断服务程序
   注意,读取USARTx->SR能避免莫名其妙的错误
*/   	
u8 USART2_RX_BUF[64];     //接收缓冲,最大64个字节.

/* 接收状态:
	bit7 - 接收完成标志;
	bit6 - 接收到0x0d;
	bit5~0 - 接收到的有效字节数目
*/
u8 USART2_RX_STA = 0;       //接收状态标记	  
  
void USART2_IRQHandler(void)
{
	u8 res;	    
	if (USART2->SR & (1<<5))//接收到数据
	{	 
		res = USART2->DR; 
		if ((USART2_RX_STA & 0x80) == 0)	//接收未完成
		{
			if (USART2_RX_STA & 0x40)	//接收到了0x0d
			{
				if (res != 0x0a)
					USART2_RX_STA = 0;	//接收错误,重新开始
				else
					USART2_RX_STA |= 0x80;	//接收完成了 
			}
			else 	//还没收到0X0D
			{	
				if (res == 0x0d)
					USART2_RX_STA |= 0x40;
				else
				{
					USART2_RX_BUF[USART2_RX_STA & 0X3F] = res;
					USART2_RX_STA++;
					if (USART2_RX_STA > 63)
						USART2_RX_STA = 0;	//接收数据错误,重新开始接收	  
				}		 
			}
		}  		 									     
	}  											 
}

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
