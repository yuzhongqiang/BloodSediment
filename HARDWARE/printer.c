/*
* Printer
*
* printer.c
*
*/

#include <stm32f10x_lib.h>
#include <stdio.h>
#include "sys.h"
#include "printer.h"
#include "channel.h"
#include "delay.h"

extern	struct tube tubes[MAX_CHANNELS];

/*
	USART3�жϷ������
	ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
*/

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
}

u8 print_ch(char ch)
{
	while ((USART3->SR & 0x40) == 0)	//�ȴ����߿���
		;

	USART3->DR = ch;      
	return ch;
}

/* str������\0�ַ���β */
void print_str(char* str)
{
 	while (*str)
		print_ch(*str++);
}

void printer_main(void)
{
	u8 i, j;
      char buf[100];

	for (i=0; i<MAX_CHANNELS; i++)
	{
		if (tubes[i].status == CHN_STATUS_FINISH)
		{
		    print_str("*************************\n");

                for (j=0; j<13; j++)
                {
                    sprintf(buf, "ESR_NO.%d_Time%d: %d\n", i, j+1, tubes[i].values[j]/2);
					print_str(buf);
                }

            print_str("*************************\n");
			delay_ms(1000);
			tubes[i].status = CHN_STATUS_NONE;
		}
            else
                continue;
	}
}

void printer_test(void)
{
 	char *str = "Hello, this is a test";
	
	print_str(str);
	print_ch(0x0a);
}
