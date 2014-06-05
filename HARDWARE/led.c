#include <stm32f10x_lib.h>	   
#include "led.h"
	   

//��ʼ��PA8��PD2Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void led_init(void)
{
	RCC->APB2ENR|=1<<2;    //ʹ��PORTAʱ��	   	 
	RCC->APB2ENR|=1<<5;    //ʹ��PORTDʱ��	
	   	 
	GPIOA->CRH&=0XFFFFFFF0; 
	GPIOA->CRH|=0X00000003;//PA8 �������   	 
    GPIOA->ODR|=1<<8;      //PA8 �����
											  
	GPIOD->CRL&=0XFFFFF0FF;
	GPIOD->CRL|=0X00000300;//PD.2�������
	GPIOD->ODR|=1<<2;      //PD.2����� 
}





