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
		   
	RCC->APB2ENR |= 1 << 2;   //ʹ��PORTA��ʱ��  OK
	RCC->APB1ENR |= 1 << 17;  	//ʹ�ܴ���2ʱ�� 
	GPIOA->CRL &= 0XFFFF00FF; 
	GPIOA->CRL |= 0X00008B00;	//IO״̬����	OK
		  
	RCC->APB1RSTR |= (1 << 17); //��λ����2	   OK
	RCC->APB1RSTR &= ~(1<<17);	//ֹͣ��λ	   OK  

	//����������
	temp = (float)(PCLK1_FREQ / (baud * 16));	//�õ�USARTDIV
	mantissa = temp;						 	//�õ���������
	fraction = (temp - mantissa) * 16; 			//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART2->BRR = mantissa; // ����������	 
	USART2->CR1 |= 0X200C;  //1λֹͣ,��У��λ.

#ifdef EN_USART2_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART2_IRQChannel, 2);//��2��������ȼ� 
#endif
}
