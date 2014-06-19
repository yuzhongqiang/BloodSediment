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
#ifdef EN_USART3_RX
static u8 USART_RX_BUF[64];

/*
	USART_RX_STAָʾ����״̬
	bit[7]:  	������ɱ�־
	bit[6]:  	���յ�0x0d
	bit[5~0]:	���յ�����Ч�ֽ���Ŀ
*/     
static u8 USART_RX_STA = 0;       //����״̬���	  

void USART1_IRQHandler(void)
{
	u8 data;	    

	if (USART3->SR & (1 << 5))			// ���յ�����
	{	 
		data = USART3->DR; 
		if ((USART_RX_STA & 0x80) == 0)	// ����δ���
		{
			if (USART_RX_STA & 0x40)	// ���յ���0x0d
			{
				if (data != 0x0a)
					USART_RX_STA = 0;		// ���մ���,���¿�ʼ
				else
					USART_RX_STA |= 0x80;	// ��������� 
			}
			else //��û�յ�0X0D
			{	
				if (data == 0x0d)
					USART_RX_STA |= 0x40;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0x3F] = data;
					USART_RX_STA++;
					if (USART_RX_STA > 63)
						USART_RX_STA = 0;	//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  		 									     
	}  											 
} 
#endif

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

#ifdef EN_USART3_RX		  	//���ʹ���˽���
	//ʹ�ܽ����ж�
	USART3->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART3->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART3_IRQChannel, 2);	//��2��������ȼ� 
#endif
}

u8 do_print(u8 ch)
{
	while ((USART3->SR & 0x40) == 0)	//�ȴ����߿���
		;

	USART3->DR = ch;      
	return ch;
}

