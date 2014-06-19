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
#ifdef EN_USART3_RX
static u8 USART_RX_BUF[64];

/*
	USART_RX_STA指示接收状态
	bit[7]:  	接收完成标志
	bit[6]:  	接收到0x0d
	bit[5~0]:	接收到的有效字节数目
*/     
static u8 USART_RX_STA = 0;       //接收状态标记	  

void USART1_IRQHandler(void)
{
	u8 data;	    

	if (USART3->SR & (1 << 5))			// 接收到数据
	{	 
		data = USART3->DR; 
		if ((USART_RX_STA & 0x80) == 0)	// 接收未完成
		{
			if (USART_RX_STA & 0x40)	// 接收到了0x0d
			{
				if (data != 0x0a)
					USART_RX_STA = 0;		// 接收错误,重新开始
				else
					USART_RX_STA |= 0x80;	// 接收完成了 
			}
			else //还没收到0X0D
			{	
				if (data == 0x0d)
					USART_RX_STA |= 0x40;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0x3F] = data;
					USART_RX_STA++;
					if (USART_RX_STA > 63)
						USART_RX_STA = 0;	//接收数据错误,重新开始接收	  
				}		 
			}
		}  		 									     
	}  											 
} 
#endif

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

#ifdef EN_USART3_RX		  	//如果使能了接收
	//使能接收中断
	USART3->CR1 |= (1 << 8);    //PE中断使能
	USART3->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	    	
	nvic_init(3, 3, USART3_IRQChannel, 2);	//组2，最低优先级 
#endif
}

u8 do_print(u8 ch)
{
	while ((USART3->SR & 0x40) == 0)	//等待总线空闲
		;

	USART3->DR = ch;      
	return ch;
}

