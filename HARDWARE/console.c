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


/* 当前接收到的命令*/
u8 g_console_curstat = CONSOLE_STAT_INIT;

/* 
    串口2中断服务程序
    读取USARTx->SR能 避免莫名其妙的错误
*/   	
void USART1_IRQHandler(void)
{
	u8 res;	 
	
	if (USART1->SR & (1<<5)) {  	//接收到数据
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
	
	RCC->APB2ENR |= 1<<2;   //使能PORTA口时钟   OK
	RCC->APB2ENR |= 1<<14;  //使能串口时钟 	  OK
	GPIOA->CRH &= 0XFFFFF00F; 
	GPIOA->CRH |= 0X000008B0;//IO状态设置
		  
	RCC->APB2RSTR |= 1<<14;   //复位串口1
	RCC->APB2RSTR &= ~(1<<14);//停止复位	   	   

	//波特率设置
	temp = (float)(PCLK2_FREQ/(baud*16));//得到USARTDIV
	mantissa = temp;				 		//得到整数部分
	fraction = (temp-mantissa)*16; 		//得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART1->BRR = mantissa; // 波特率设置	 
	USART1->CR1 |= 0X200C;  //1位停止,无校验位.

	//使能接收中断
	USART1->CR1 |= (1 << 8);	  //PE中断使能
	USART1->CR1 |= (1 << 5);	  //接收缓冲区非空中断使能			
	nvic_init(3, 3, USART1_IRQChannel, 2);//组2，最低优先级 
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

			/* 存储刷卡值*/
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
	while ((USART1->SR & 0x40) == 0)	//等待总线空闲
		;

	USART1->DR = ch;      
	return ch;
}

/* str必须以\0字符结尾 */
void console_send_str(u8* str)
{
 	while (*str)
		console_send_ch(*str++);
} 

