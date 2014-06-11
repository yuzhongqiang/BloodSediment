#include "sys.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/5/27
//�汾��V1.3
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#ifdef EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[64];     //���ջ���,���64���ֽ�.
//����״̬
//bit7��������ɱ�־
//bit6�����յ�0x0d
//bit5~0�����յ�����Ч�ֽ���Ŀ
u8 USART_RX_STA=0;       //����״̬���	  
  
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
//��ʼ��IO ����1
//baud:������
/*
  Baudrate = fck/(16*USARTDIV)	; Baudrate is of float type



*/
void uart1_init(u32 baud)
{  	 
	float temp;
	u16 mantissa;
	u16 fraction;
		   
	temp = (float)(PCLK2_FREQ/(baud*16));//�õ�USARTDIV
	mantissa=temp;				 		//�õ���������
	fraction=(temp-mantissa)*16; 		//�õ�С������	 
    mantissa<<=4;
	mantissa+=fraction; 
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��  
	RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRH&=0XFFFFF00F; 
	GPIOA->CRH|=0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ	   	   
	//����������
 	USART1->BRR=mantissa; // ����������	 
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ.
#ifdef EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART1->CR1|=1<<8;    //PE�ж�ʹ��
	USART1->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3,3,USART1_IRQChannel,2);//��2��������ȼ� 
#endif
}

void uart2_init(u32 baud)
{  	 
	float temp;
	u16 mantissa, fraction;
		   
	//RCC->APB2ENR |= 1 << 2;   //ʹ��PORTA��ʱ��  ???
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

#ifdef EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART2_IRQChannel, 2);//��2��������ȼ� 
#endif
}

void uart3_init(u32 baud)
{  	 
	float temp;
	u16 mantissa, fraction;
		   
	//RCC->APB2ENR |= 1 << 3;   //ʹ��PORTB��ʱ��  ???
	RCC->APB1ENR |= 1 << 18;  	//ʹ�ܴ���ʱ��  OK
	//GPIOA->CRH &= 0XFFFFF00F; 
	GPIOA->CRH |= 0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR |= (1 << 14);   //��λ����1
	RCC->APB2RSTR &= ~(1 << 14);//ֹͣ��λ	   	   

	//����������
	temp = (float)(PCLK1_FREQ / (baud * 16));	//�õ�USARTDIV
	mantissa = temp;						 	//�õ���������
	fraction = (temp - mantissa) * 16; 			//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART3->BRR = mantissa; // ����������	 
	USART3->CR1 |= 0X200C;  //1λֹͣ,��У��λ.

#ifdef EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART3_IRQChannel, 2);//��2��������ȼ� 
#endif
}

#if 0
void uart4_init(u32 baud)
{  	 
	float temp;
	u16 mantissa, fraction;
		   
	//RCC->APB2ENR |= 1 << 2;   //ʹ��PORTA��ʱ��  ???
	RCC->APB1ENR |= 1 << 19;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRH &= 0XFFFFF00F; 
	GPIOA->CRH |= 0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR |= (1 << 14);   //��λ����1
	RCC->APB2RSTR &= ~(1 << 14);//ֹͣ��λ	   	   

	//����������
	temp = (float)(PCLK1_FREQ / (baud * 16));	//�õ�USARTDIV
	mantissa = temp;						 	//�õ���������
	fraction = (temp - mantissa) * 16; 			//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART4->BRR = mantissa; // ����������	 
	USART4->CR1 |= 0X200C;  //1λֹͣ,��У��λ.

#ifdef EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART4->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART4->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART4_IRQChannel, 2);//��2��������ȼ� 
#endif
}


void uart5_init(u32 baud)
{  	 
	float temp;
	u16 mantissa, fraction;
		   
	//RCC->APB2ENR |= 1 << 2;   //ʹ��PORTA��ʱ��  ???
	RCC->APB1ENR |= 1 << 20;  //ʹ�ܴ���ʱ�� 
	GPIOA->CRH &= 0XFFFFF00F; 
	GPIOA->CRH |= 0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR |= (1 << 14);   //��λ����1
	RCC->APB2RSTR &= ~(1 << 14);//ֹͣ��λ	   	   

	//����������
	temp = (float)(PCLK1_FREQ / (baud * 16));	//�õ�USARTDIV
	mantissa = temp;						 	//�õ���������
	fraction = (temp - mantissa) * 16; 			//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART5->BRR = mantissa; // ����������	 
	USART5->CR1 |= 0X200C;  //1λֹͣ,��У��λ.

#ifdef EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART5_IRQChannel, 2);//��2��������ȼ� 
#endif
}
#endif
