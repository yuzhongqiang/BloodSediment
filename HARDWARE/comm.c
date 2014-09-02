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
<<<<<<< HEAD
u8 UART4_RX_BUF[64];     //���ջ���,���64���ֽ�.
=======
u8 USART4_RX_BUF[64];     //���ջ���,���64���ֽ�.
>>>>>>> 06b38c2d4242cfd2cbbed58288137ca6943eccbc

/* ����״̬:
	bit7 - ������ɱ�־;
	bit6 - ���յ�0x0d;
	bit5~0 - ���յ�����Ч�ֽ���Ŀ
*/
<<<<<<< HEAD
u8 UART4_RX_STA = 0;       //����״̬���	  
  
void UART4_IRQHandler(void)
=======
u8 USART4_RX_STA = 0;       //����״̬���	  
  
void USART4_IRQHandler(void)
>>>>>>> 06b38c2d4242cfd2cbbed58288137ca6943eccbc
{
	u8 res;	    
	if (UART4->SR & (1<<5))//���յ�����
	{	 
		res = UART4->DR; 
<<<<<<< HEAD
		if ((UART4_RX_STA & 0x80) == 0)	//����δ���
		{
			if (UART4_RX_STA & 0x40)	//���յ���0x0d
			{
				if (res != 0x0a)
					UART4_RX_STA = 0;	//���մ���,���¿�ʼ
				else
					UART4_RX_STA |= 0x80;	//��������� 
=======
		if ((USART4_RX_STA & 0x80) == 0)	//����δ���
		{
			if (USART4_RX_STA & 0x40)	//���յ���0x0d
			{
				if (res != 0x0a)
					USART4_RX_STA = 0;	//���մ���,���¿�ʼ
				else
					USART4_RX_STA |= 0x80;	//��������� 
>>>>>>> 06b38c2d4242cfd2cbbed58288137ca6943eccbc
			}
			else 	//��û�յ�0X0D
			{	
				if (res == 0x0d)
<<<<<<< HEAD
					UART4_RX_STA |= 0x40;
				else
				{
					UART4_RX_BUF[UART4_RX_STA & 0X3F] = res;
					UART4_RX_STA++;
					if (UART4_RX_STA > 63)
						UART4_RX_STA = 0;	//�������ݴ���,���¿�ʼ����	  
=======
					USART4_RX_STA |= 0x40;
				else
				{
					USART4_RX_BUF[USART4_RX_STA & 0X3F] = res;
					USART4_RX_STA++;
					if (USART4_RX_STA > 63)
						USART4_RX_STA = 0;	//�������ݴ���,���¿�ʼ����	  
>>>>>>> 06b38c2d4242cfd2cbbed58288137ca6943eccbc
				}		 
			}
		}  		 									     
	}  											 
}

void comm_init(u32 baud)
{  	 
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
 	UART4->BRR = mantissa; // ����������	 
	UART4->CR1 |= 0X200C;  // 1λֹͣ,��У��λ.

	//ʹ�ܽ����ж�
<<<<<<< HEAD
	UART4->CR1 |= (1 << 8);    //PE�ж�ʹ��
	UART4->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, UART4_IRQChannel, 2);//��2��������ȼ� 
=======
	USART4->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART4->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART4_IRQChannel, 2);//��2��������ȼ� 
#endif
>>>>>>> 06b38c2d4242cfd2cbbed58288137ca6943eccbc
}

