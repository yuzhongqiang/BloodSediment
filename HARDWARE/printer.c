/*
* Printer
*
* printer.c
*
*/

#include <stm32f10x_lib.h>
#include <stdio.h>
#include "sys.h"
#include "printer.h"
#include "channel.h"
#include "delay.h"

extern	struct tube tubes[MAX_CHANNELS];

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

u8 print_ch(char ch)
{
	while ((USART3->SR & 0x40) == 0)	//等待总线空闲
		;

	USART3->DR = ch;      
	return ch;
}

/* str必须以\0字符结尾 */
void print_str(char* str)
{
 	while (*str)
		print_ch(*str++);
}

void printer_main(void)
{
	u8 i, j;
      char buf[100];

	for (i=0; i<MAX_CHANNELS; i++)
	{
		if (tubes[i].status == CHN_STATUS_FINISH)
		{
		    print_str("*************************\n");

                for (j=0; j<13; j++)
                {
                    sprintf(buf, "ESR_NO.%d_Time%d: %d\n", i, j+1, tubes[i].values[j]/2);
					print_str(buf);
                }

            print_str("*************************\n");
			delay_ms(1000);
			tubes[i].status = CHN_STATUS_NONE;
		}
            else
                continue;
	}
}

void printer_test(void)
{
 	char *str = "Hello, this is a test";
	
	print_str(str);
	print_ch(0x0a);
}
