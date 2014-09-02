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

/* 串口2中断服务程序
   注意,读取USARTx->SR能避免莫名其妙的错误
*/   	
u8 UART4_RX_BUF[64];     //接收缓冲,最大64个字节.

/* 接收状态:
	bit7 - 接收完成标志;
	bit6 - 接收到0x0d;
	bit5~0 - 接收到的有效字节数目
*/
u8 UART4_RX_STA = 0;       //接收状态标记
  
void UART4_IRQHandler(void)	
{
	u8 res;	    
	if (UART4->SR & (1<<5))//接收到数据
	{	 
		res = UART4->DR; 
		if ((UART4_RX_STA & 0x80) == 0)	//接收未完成
		{
			if (UART4_RX_STA & 0x40)	//接收到了0x0d
			{
				if (res != 0x0a)
					UART4_RX_STA = 0;	//接收错误,重新开始
				else
					UART4_RX_STA |= 0x80;	//接收完成了 
			}
			else 	//还没收到0X0D
			{	
				if (res == 0x0d)
					UART4_RX_STA |= 0x40;
				else
				{
					UART4_RX_BUF[UART4_RX_STA & 0X3F] = res;
					UART4_RX_STA++;
					if (UART4_RX_STA > 63)
						UART4_RX_STA = 0;	//接收数据错误,重新开始接收	  
				}		 
			}
		}  		 									     
	}  											 
}

void comm_init(u32 baud)
{  	 
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
 	UART4->BRR = mantissa; // 波特率设置	 
	UART4->CR1 |= 0X200C;  // 1位停止,无校验位.

	//使能接收中断
	UART4->CR1 |= (1 << 8);    //PE中断使能
	UART4->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	    	
	nvic_init(3, 3, UART4_IRQChannel, 2);//组2，最低优先级 
}

u8 send_ch(u8 ch)
{
	while ((UART4->SR & 0x40) == 0)	//等待总线空闲
		;

	UART4->DR = ch;      
	return ch;
}

/* str必须以\0字符结尾 */
void send_str(u8* str)
{
 	while (*str++)
		print_ch(*str);
}
