/*
* LCD
*
* lcd.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "lcd.h"


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
