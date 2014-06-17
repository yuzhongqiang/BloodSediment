/*
* Printer
*
* printer.c
*
*/

#include <stm32f10x_lib.h>
#include "printer.h"



/* ��ӡ��ʹ��USART3 */
void printer_init(void)
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
	USART3->CR1 |= 0X200C;  //1λֹͣ,��У��λ.

#ifdef EN_USART3_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART3->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART3->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART3_IRQChannel, 2);//��2��������ȼ� 
#endif
}

