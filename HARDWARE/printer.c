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
	USART3�жϷ������
	ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
*/

/* ��ӡ��ʹ��USART3 */
void printer_init(u32 baud)
{
	float temp;
	u16 mantissa, fraction;
		   
	RCC->APB1ENR |= 1 << 18;  	//ʹ�ܴ���ʱ��  OK
	GPIOB->CRH &= 0XFFFF00FF; 
	GPIOB->CRH |= 0X00008B00;	//IO״̬����  OK
		  
	RCC->APB1RSTR |= (1 << 18);   //��λ����3	 OK
	RCC->APB1RSTR &= ~(1 << 18);//ֹͣ��λ	   	  OK

	//����������
	temp = (float)(PCLK1_FREQ / (baud * 16));	//�õ�USARTDIV
	mantissa = temp;						 	//�õ���������
	fraction = (temp - mantissa) * 16; 			//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART3->BRR = mantissa; // ����������	 
	USART3->CR1 |= 0X200C;  // 1λֹͣ,��У��λ.
}

u8 print_ch(u8 ch)
{
	while ((USART3->SR & 0x40) == 0)	//�ȴ����߿���
		;

	USART3->DR = ch;      
	return ch;
}

/* str������\0�ַ���β */
void print_str(u8* str)
{
 	while (*str)
		print_ch(*str++);
}
