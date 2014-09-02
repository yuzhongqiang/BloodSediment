/*
* LCD
*
* lcd.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "lcd.h"

<<<<<<< HEAD

=======
>>>>>>> 06b38c2d4242cfd2cbbed58288137ca6943eccbc
/*
  LCD uses USART1
  Baudrate = fck/(16*USARTDIV)	; Baudrate is of float type
*/
void lcd_init(u32 baud)
{
	float temp;
	u16 mantissa;
	u16 fraction;
		   
	RCC->APB2ENR|=1<<2;   //使能PORTA口时钟   OK
	RCC->APB2ENR|=1<<14;  //使能串口时钟 	  OK
	GPIOA->CRH&=0XFFFFF00F; 
	GPIOA->CRH|=0X000008B0;//IO状态设置
		  
	RCC->APB2RSTR|=1<<14;   //复位串口1
	RCC->APB2RSTR&=~(1<<14);//停止复位	   	   

	//波特率设置
	temp = (float)(PCLK2_FREQ/(baud*16));//得到USARTDIV
	mantissa=temp;				 		//得到整数部分
	fraction=(temp-mantissa)*16; 		//得到小数部分	 
    mantissa<<=4;
	mantissa+=fraction; 
 	USART1->BRR=mantissa; // 波特率设置	 
	USART1->CR1|=0X200C;  //1位停止,无校验位.
}
