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


/* 串口2中断服务程序
   注意,读取USARTx->SR能避免莫名其妙的错误
*/   	
u8 g_console_rxbuf[64];
u8 g_console_rxcnt = 0;

//接收状态
#define _STATE_RECIEVING  0
#define _STATE_0XD_RECVED  1
u8 g_console_rxstat = _STATE_RECIEVING;

/* 当前接收到的命令*/
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

			
			sprintf(str, "mng_lbl_value.text=剩余次数:%d\n", remain); 
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
	
	if (USART1->SR & (1<<5))		//接收到数据
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

u8 console_recv_cmd(void)
{
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



