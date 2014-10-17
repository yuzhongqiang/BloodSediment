/*
* LCD
*
* lcd.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "console.h"
#include "channel.h"
#include "storage.h"

/* Console command format 
Total length:8bytes
Header(2bytes): 0xf3, 0xd7
cmd(4bytes): page#(1byte), cmd#(1byte), resv, resv
Tail(2bytes): 0x0d, 0x0a
Page 1:
	cmd1: run
	cmd2: pause
	cmd3: status
Page 2:
	cmd1: query
	cmd2: buy
	
For example:
0xf3,0xd7,0x00,0x01,0x00,0x00,0x0d,0x0a
This is the run command in page 1
*/


/* ����2�жϷ������
   ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
*/   	
u8 g_console_rxbuf[64];
u8 g_console_rxcnt = 0;

//����״̬
#define _STATE_RECIEVING  0
#define _STATE_0XD_RECVED  1
u8 g_console_rxstat = _STATE_RECIEVING;

/* ��ǰ���յ�������*/
u8 g_console_curstat = CONSOLE_STAT_INIT;

u8 _console_parse(void)
{
	u32 remain = 0;
	char str[64];

	memset(str, 0, sizeof(64));
	switch (g_console_rxbuf[2])
	{
	case 0:
		break;
	case 1:   //main page		
		if (0x01 == g_console_rxbuf[3])      /* Run */
		{
			channel_resume();
			g_console_curstat= CONSOLE_STAT_RUNNING;
		}
		else if (0x02 == g_console_rxbuf[3]) /* Pause */
		{
			channel_pause();
			g_console_curstat = CONSOLE_STAT_PAUSE;
		}
		else if (0x03 == g_console_rxbuf[3]) /* Enter manage page */
		{
			channel_pause();
			g_console_curstat = CONSOLE_STAT_MNG;
		}
		break;
	case 2:  // manage page
		if (0x01 == g_console_rxbuf[3])  /* Query remain */
		{
			remain = storage_query();
			sprintf(str, "mng_lbl_remain.text=%d\n", remain);
			console_send_str(str);
		}
		else if (0x02 == g_console_rxbuf[3])  /* Buy license */
		{
			// Buy license
			reader_change_cc(4);

			
			sprintf(str, "mng_lbl_value.text=ʣ�����:%d\n", remain); 
			storage_save(0x1234);
		}
		else if (0x03 == g_console_rxbuf[3])  /* Return to main page */
		{
			channel_pause();
			g_console_curstat = CONSOLE_STAT_MNG;
		}
		break;
	default:
		break;
	}
	return 0;
}
  
void USART1_IRQHandler(void)
{
{
	u8 res;	 
	
	if (USART1->SR & (1<<5))		//���յ�����
	{	 
    		res = USART1->DR; 
		switch (g_console_rxstat)
		{
		case _STATE_RECIEVING:
			if (0x0d == res)
			{
				g_console_rxbuf[g_console_rxcnt++] = res;
				g_console_rxstat = _STATE_0XD_RECVED;
			}
			else
				g_console_rxbuf[g_console_rxcnt++] = res;			
			break;
		case _STATE_0XD_RECVED:
			if (res != 0x0A)     //error, restart
			{
				g_console_rxcnt = 0;
				g_console_rxstat = _STATE_RECIEVING;
			}
			else
			{
			   g_console_rxbuf[g_console_rxcnt++] = res;
			   if (g_console_rxbuf[0] == 0xf3 && g_console_rxbuf[1] == 0xd7
			   	&& g_console_rxcnt == 8)
			   {
				_console_parse();	
			   }
				g_console_rxcnt = 0;
				g_console_rxstat = _STATE_RECIEVING;
			}
			break;
		}
	}  											 
} 										 
}

/*
  LCD uses USART1
  Baudrate = fck/(16*USARTDIV)	; Baudrate is of float type
*/
void console_init(u32 baud)
{
	float temp;
	u16 mantissa;
	u16 fraction;
		   
	RCC->APB2ENR |= 1<<2;   //ʹ��PORTA��ʱ��   OK
	RCC->APB2ENR |= 1<<14;  //ʹ�ܴ���ʱ�� 	  OK
	GPIOA->CRH &= 0XFFFFF00F; 
	GPIOA->CRH |= 0X000008B0;//IO״̬����
		  
	RCC->APB2RSTR |= 1<<14;   //��λ����1
	RCC->APB2RSTR &= ~(1<<14);//ֹͣ��λ	   	   

	//����������
	temp = (float)(PCLK2_FREQ/(baud*16));//�õ�USARTDIV
	mantissa = temp;				 		//�õ���������
	fraction = (temp-mantissa)*16; 		//�õ�С������	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART1->BRR = mantissa; // ����������	 
	USART1->CR1 |= 0X200C;  //1λֹͣ,��У��λ.

	//ʹ�ܽ����ж�
	USART1->CR1 |= (1 << 8);	  //PE�ж�ʹ��
	USART1->CR1 |= (1 << 5);	  //���ջ������ǿ��ж�ʹ��			
	nvic_init(3, 3, USART1_IRQChannel, 2);//��2��������ȼ� 
}

u8 console_recv_cmd(void)
{
	return g_console_curstat;
}

u8 console_send_ch(u8 ch)
{
	while ((USART1->SR & 0x40) == 0)	//�ȴ����߿���
		;

	USART1->DR = ch;      
	return ch;
}

/* str������\0�ַ���β */
void console_send_str(u8* str)
{
 	while (*str)
		console_send_ch(*str++);
}



