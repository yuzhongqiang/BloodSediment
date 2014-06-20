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
void comm_init(u32 baud)
{  	 
#if 0
	float temp;
	u16 mantissa, fraction;
		   
	RCC->APB2ENR |= 1 << 4;   	//ʹ��PORTC��ʱ��  OK
	RCC->APB1ENR |= 1 << 19;  	//ʹ�ܴ���ʱ�� 		OK
	GPIOC->CRH &= 0XFFFF00FF; 
	GPIOC->CRH |= 0X00008B00;		//IO״̬����	 OK
		  
	RCC->APB1RSTR |= (1 << 19);   	//��λ����4		OK
	RCC->APB1RSTR &= ~(1 << 19);	//ֹͣ��λ	   	OK 

	//����������
	temp = (float)(PCLK1_FREQ / (baud * 16));	//�õ�USARTDIV
	mantissa = temp;						 	//�õ���������
	fraction = (temp - mantissa) * 16; 			//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART4->BRR = mantissa; // ����������	 
	USART4->CR1 |= 0X200C;  // 1λֹͣ,��У��λ.

#ifdef EN_USART4_RX		  	//���ʹ���˽���
	//ʹ�ܽ����ж�
	USART4->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART4->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART4_IRQChannel, 2);//��2��������ȼ� 
#endif
#endif
}

