/*
* LCD
*
* lcd.c
*
*/

#include <stm32f10x_lib.h>
#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "console.h"
#include "channel.h"
#include "storage.h"
#include "reader.h"
#include "delay.h"
#include "storage.h"
#include "buffer.h"

struct circle_buf g_cons_buf;
extern struct _card_info card_info;


/* ��ǰ���յ�������*/
u8 g_console_curstat = CONSOLE_STAT_INIT;

/* 
    ����2�жϷ������
    ��ȡUSARTx->SR�� ����Ī������Ĵ���
*/   	
void USART1_IRQHandler(void)
{
	u8 res;	 
	
	if (USART1->SR & (1<<5)) {  	//���յ�����
		res = USART1->DR;
		buffer_push_byte(&g_cons_buf, res);
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

	buffer_clear(&g_cons_buf);
	
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
u8 console_main(void)
{	
	u16 remain;
	u8 ch1, ch2;
	char str[100];

	memset(str, 0, 100);
loop:
	/* find frame head */
	while (((buffer_size(&g_cons_buf)) > 0) && (BUF_HEAD(g_cons_buf) != 0xf3))
		buffer_pop_byte(&g_cons_buf);
	if (buffer_size(&g_cons_buf)==0)
		return g_console_curstat;

	/* now frame head is 0xf3 */
	buffer_pop_byte(&g_cons_buf);
	if (BUF_HEAD(g_cons_buf) != 0xd7) // ???
		return g_console_curstat;
	buffer_pop_byte(&g_cons_buf);
	
	if (buffer_size(&g_cons_buf)<6)    // ???
		return g_console_curstat;
	/* frame tail check */
	if ((buffer_chk_nbyte(&g_cons_buf, 4)!= 0x0d) ||
		(buffer_chk_nbyte(&g_cons_buf, 5) != 0x0a)) {
		/* frame tail error */
		buffer_pop_nbytes(&g_cons_buf, 6);
		goto loop;
	}

	/* now parse the commands */
	ch1 = buffer_chk_nbyte(&g_cons_buf, 0);
	ch2 = buffer_chk_nbyte(&g_cons_buf, 1);
	switch (ch1) {
	case 0:
		break;
	case 1:   //main page		
		if (0x01 == ch2) {    /* Run */
			channel_resume();
			g_console_curstat= CONSOLE_STAT_RUNNING;
		}
		else if (0x02 == ch2) {  /* Pause */
			channel_pause();
			g_console_curstat = CONSOLE_STAT_PAUSE;
		}
		else if (0x03 == ch2) {  /* Enter manage page */
			channel_pause();
			g_console_curstat = CONSOLE_STAT_MNG;
		}
		break;
	case 2:  // manage page
		if (0x01 == ch2) {      /* Query remain */
			g_console_curstat = CONSOLE_STAT_QUERY;
		}
		else if (0x02 == ch2) {  /* Buy license */
			g_console_curstat = CONSOLE_STAT_BUY;
		}
		else if (0x03 == ch2) {   /* Return to main page */
			channel_pause();
			g_console_curstat = CONSOLE_STAT_MNG;
		}
		break;
	default:
		break;
	}
	buffer_pop_nbytes(&g_cons_buf, 6);

	switch (g_console_curstat) {
	case CONSOLE_STAT_RUNNING:
	case CONSOLE_STAT_PAUSE:
	case CONSOLE_STAT_MNG:
		break;		
		
	case CONSOLE_STAT_BUY:
			reader_read_block(1);
			delay_ms(2000);

			sprintf(str, "mng_lbl_value.text=%d\n", card_info.value);
			console_send_str((u8*)str);

			/* �洢ˢ��ֵ*/
			storage_add(card_info.value);
		break;
	case CONSOLE_STAT_QUERY:
			remain = storage_query();
			sprintf(str, "mng_lbl_remain.text=%d\n", remain);
			console_send_str((u8*)str);
		break;

	default:
		break;		
	}
	
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

