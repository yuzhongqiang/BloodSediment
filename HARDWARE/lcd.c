/*
* LCD
*
* lcd.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "lcd.h"

#ifdef EN_USART1_RX
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
static u8 USART_RX_BUF[64];     //���ջ���,���64���ֽ�.
//����״̬
//bit7��������ɱ�־
//bit6�����յ�0x0d
//bit5~0�����յ�����Ч�ֽ���Ŀ
static u8 USART_RX_STA=0;       //����״̬���	  
  
void USART1_IRQHandler(void)
{
	u8 res;	    
	if(USART1->SR&(1<<5))//���յ�����
	{	 
		res=USART1->DR; 
		if((USART_RX_STA&0x80)==0)//����δ���
		{
			if(USART_RX_STA&0x40)//���յ���0x0d
			{
				if(res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x80;	//��������� 
			}else //��û�յ�0X0D
			{	
				if(res==0x0d)USART_RX_STA|=0x40;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3F]=res;
					USART_RX_STA++;
					if(USART_RX_STA>63)USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  		 									     
	}  											 
} 
#endif	


/*
  LCD uses USART1
  Baudrate = fck/(16*USARTDIV)	; Baudrate is of float type
*/
void lcd_init(u32 baud)
{
	float temp;
	u16 mantissa;
	u16 fraction;
		   
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��   OK
	RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ�� 	  OK
	GPIOA->CRH&=0XFFFFF00F; 
	GPIOA->CRH|=0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ	   	   

	//����������
	temp = (float)(PCLK2_FREQ/(baud*16));//�õ�USARTDIV
	mantissa=temp;				 		//�õ���������
	fraction=(temp-mantissa)*16; 		//�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
 	USART1->BRR=mantissa; // ����������	 
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ.

#ifdef EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART1->CR1|=1<<8;    //PE�ж�ʹ��
	USART1->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3,3,USART1_IRQChannel,2);//��2��������ȼ� 
#endif
}
