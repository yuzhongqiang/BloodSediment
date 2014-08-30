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

/* ����2�жϷ������
   ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
*/   	
u8 USART4_RX_BUF[64];     //���ջ���,���64���ֽ�.

/* ����״̬:
	bit7 - ������ɱ�־;
	bit6 - ���յ�0x0d;
	bit5~0 - ���յ�����Ч�ֽ���Ŀ
*/
u8 USART4_RX_STA = 0;       //����״̬���	  
  
void USART4_IRQHandler(void)
{
	u8 res;	    
	if (UART4->SR & (1<<5))//���յ�����
	{	 
		res = UART4->DR; 
		if ((USART4_RX_STA & 0x80) == 0)	//����δ���
		{
			if (USART4_RX_STA & 0x40)	//���յ���0x0d
			{
				if (res != 0x0a)
					USART4_RX_STA = 0;	//���մ���,���¿�ʼ
				else
					USART4_RX_STA |= 0x80;	//��������� 
			}
			else 	//��û�յ�0X0D
			{	
				if (res == 0x0d)
					USART4_RX_STA |= 0x40;
				else
				{
					USART4_RX_BUF[USART4_RX_STA & 0X3F] = res;
					USART4_RX_STA++;
					if (USART4_RX_STA > 63)
						USART4_RX_STA = 0;	//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  		 									     
	}  											 
}

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

	//ʹ�ܽ����ж�
	USART4->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART4->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART4_IRQChannel, 2);//��2��������ȼ� 
#endif
}

