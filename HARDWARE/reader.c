/*
* Card reader
*
* reader.c
*
*/

#include <stm32f10x_lib.h>
#include "sys.h"
#include "reader.h"
#include "delay.h"

struct _card_info card_info;

/* IC Reader uses USART2 */

/* 存放充值数据的快号 */
#define VALUE_BLOCK (0x04)

/* 串口2中断服务程序
   注意,读取USARTx->SR能避免莫名其妙的错误
*/   	
u8 g_reader_rxbuf[64];     //接收缓冲,最大64个字节.
u8 g_reader_rxcnt = 0;

#if 0
void _reader_enable_intr(void)
{
	g_reader_rxcnt = 0;
	USART2->CR1 |= (1 << 8);    //PE中断使能
	USART2->CR1 |= (1 << 5);    //接收缓冲区非空中断使�
}

void _reader_disable_intr(void)
{
	USART2->CR1 &= ~(1 << 8);    //PE中断使能
	USART2->CR1 &= ~(1 << 5);    //接收缓冲区非空中断使
}
#endif

void USART2_IRQHandler(void)
{
	u8 res;	
	
	if (USART2->SR & (1<<5))//接收到数据
	{	 
		res = USART2->DR; 
		g_reader_rxbuf[g_reader_rxcnt] = res;
		g_reader_rxcnt++;
	}  	

	if ((g_reader_rxbuf[g_reader_rxbuf[0]-1] == 0x03 )&& (g_reader_rxcnt > 0)
		&& (g_reader_rxbuf[0] == g_reader_rxcnt))
	{
		switch (g_reader_rxbuf[1])
		{
			case 0x01:   //Card info
				card_info.present = 1;
				card_info.status = g_reader_rxbuf[2];
				break;
	
			case 0x02:   /* 一键读块或写卡*/
				if (g_reader_rxcnt > 6)     /* 读卡*/
					card_info.value = ((g_reader_rxbuf[4] << 8) + g_reader_rxbuf[5]);
				else     /* 写卡*/
					;
				break;
				
			default:
				break;
		}
		g_reader_rxcnt = 0;
	}
}

void reader_init(u32 baud)
{
	float temp;
	u16 mantissa, fraction;
		   
	RCC->APB2ENR |= 1 << 2;   //使能PORTA口时钟  OK
	RCC->APB1ENR |= 1 << 17;  	//使能串口2时钟 
	GPIOA->CRL &= 0XFFFF00FF; 
	GPIOA->CRL |= 0X00008B00;	//IO状态设置	OK
		  
	RCC->APB1RSTR |= (1 << 17); //复位串口2	   OK
	RCC->APB1RSTR &= ~(1<<17);	//停止复位	   OK  

	//波特率设置
	temp = (float)(PCLK1_FREQ / (baud * 16));	//得到USARTDIV
	mantissa = temp;						 	//得到整数部分
	fraction = (temp - mantissa) * 16; 			//得到小数部分	 
    mantissa <<= 4;
	mantissa += fraction; 
 	USART2->BRR = mantissa; // 波特率设置	 
	USART2->CR1 |= 0X200C;  //1位停止,无校验位.

	//使能接收中断
	USART2->CR1 |= (1 << 8);    //PE中断使能
	USART2->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	    	
	nvic_init(3, 3, USART2_IRQChannel, 2);//组2，最低优先级 
}

static u8 _send_byte(u8 ch)
{
	while ((USART2->SR & 0x40) == 0)	//等待总线空闲
		;

	USART2->DR = ch;      
	return ch;
}

void reader_send_bytes(u8* str, u8 len)
{
	u8 i;
	for (i=0; i<len; i++)
		_send_byte(str[i]);
}

static u8 reader_fill_checksum(u8* buf, u8 len)
{
	u8 temp = 0, i;
	
	for (i = 0; i < len - 2; i++)
	{
		temp ^= buf[i];
	}
	buf[len-2] = ~temp; 

	return ~temp;
}

/*
[FrameLen]	[SEQ/CmdType]	[Cmd/Status]	[Length]	[Info] 	[BCC]	[ETX]
1byte          	1byte   			1byte  			1byte  	N bytes  1byte  	1byte
*/
#if 0
void reader_get_cardinfo(void)
{
	//u8 buf[6] = {0x06, 0x01, 0x41, 0x00, 0xB9/*checksum*/, 0x03};  //for test
	u8 buf[6] = {0x06, 0x01, 0x41, 0x00, 0x00/*checksum*/, 0x03};
	reader_fill_checksum(buf, 6);
	reader_send_bytes(buf, sizeof(buf));	
}
#endif

void reader_close_card(void)
{
	u8 buf[6] = {0x06, 0x02, 0x44, 0x00, 0x00, 0x03};
	reader_fill_checksum(buf, 6);
	reader_send_bytes(buf, sizeof(buf));
}

u16 reader_read_block(u8 blk)
{
	u8 buf[15] = {0x0F, 0x02, 0x52, 0x09, 0x00/*blkno*/, 0x01, 0x60/*use keyA*/, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x03};
	buf[4] = blk;
	reader_fill_checksum(buf, 15);
	reader_send_bytes(buf, sizeof(buf));
	delay_ms(1000);
	return 0;
}

void reader_write_block(u8 blk, u16 value)
{
	u8 buf[31] = {0x1F, 0x02, 0x57,
		0x19, 0x00/*blkno*/, 0x01, 0x60/*keyA*/, 
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x03};
	buf[4] = blk;
	buf[13] = (u8)((value & 0xff00) >> 8);
	buf[14] = (u8)(value & 0x00ff);
	reader_fill_checksum(buf, sizeof(buf));
	reader_send_bytes(buf, sizeof(buf));
}

#if 0
static u8 reader_checksum(u8* data, u8 offset, u8 len)
{
	u8 temp = 0, i;
	
	for (i = offset; i < len + offset; i++)
	{
		temp ^= data[i];
	}
	return temp; 
}

static void _adjust_buf(u8 *sbuf, u8 slen, u8 *dbuf, u8 *dlen)
{
	u8 i = 0;
	u8 j = 0;

	for (i=0; i<slen; i++)
	{
		dbuf[j++] = sbuf[i];
		if (sbuf[i] == 0x7f)
			dbuf[j++] = 0x7f;
	}
	*dlen = j;
}

/*
	数据存放格式定义:
	+---------------------------------------------------------------------------------------
	| 命令头(1 byte) 	命令长度(1 byte)	命令字(1 byte)	数据(n byte)	校验(1 byte) |
	+---------------------------------------------------------------------------------------
	| 7F				2~7E				xx				…				CRC			    |	
	+---------------------------------------------------------------------------------------

	数据存放位置:
	Block4: 总充值数目
	Block7: Block4~7所在扇区的key和存取控制码:
			keyA:             0xff 0xff 0xff 0xff 0xff 0xff
			Control Code: 0xff 0x07 0x80 0x00  [Block0~2:rw  Block4:keyA(w) CC(rw) keyB(rw) -- need keyA|keyB]
			keyB:             0xff 0xff 0xff 0xff 0xff 0xff
*/

/* 改变BlockX的keyA|CC|keyB */
void reader_change_cc(u8 block)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[60];
	u8 i;
	
	u8 tmp_buf[27] = {0x7f, 0x19/*len*/, 0x15/*cmd*/,  4+3/*block7*/,
				    	/*keyB*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*data-keyA*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*data-CC*/0xff, 0x07, 0x80, 0x00,
						/*data-keyB*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*checksum*/};
	checksum = reader_checksum(tmp_buf, 2, 24);
	tmp_buf[14] = checksum;

	_adjust_buf(tmp_buf, 15, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		//等待总线空闲
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	//g_reader_rxcnt = 0;
	//reader_recv(4000);
	
}

/*  卡片一键充值
      command code: 0x12
      block address:   1byte
	验证的密钥B（6bytes）
	充值金额（4bytes�

	返回值:
		状态（1）
		卡类型（2）
		卡号（4）
		余额（4）
*/
void reader_write_value(u16 value)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[32];
	u8 i;
	
	u8 tmp_buf[15] = {0x7f, 0x0d, 0x12, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,};
	*((u32*)(tmp_buf+10)) = value;
	checksum = reader_checksum(tmp_buf, 2, 12/*13?*/);
	tmp_buf[14] = checksum;

	_adjust_buf(tmp_buf, 15, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		//等待总线空闲
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	g_reader_rxcnt = 0;
	reader_recv(4000);
}

/* 一键读块
      command code: 0x10

	返回值:
		状态（1）
		卡类型（2）
		卡号（4）
*/
u32 reader_read_cardinfo(void)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[4];
	u8 i;
	
	u8 tmp_buf[4] = {0x7f, 0x2/*len*/, 0x10/*cmd*/,/*checksum*/};
	checksum = reader_checksum(tmp_buf, 1, 2);
	tmp_buf[3] = checksum;

	_adjust_buf(tmp_buf, 4, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		//等待总线空闲
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	//g_reader_rxcnt = 0;
	//reader_recv(4000);	
}


/* 一键读块
      command code: 0x14
      block address:   1byte
	验证的密钥A（6bytes）

	返回值:
		状态（1）
		卡类型（2）
		卡号（4）
		数据(16)
*/
u32 reader_read_block(u8 block)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[60];
	u8 i;
	
	u8 tmp_buf[27] = {0x7f, 0x9/*len*/, 0x14/*cmd*/,  4/*block#*/,
				    	/*keyA*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*checksum*/};
	checksum = reader_checksum(tmp_buf, 2, 24);
	tmp_buf[14] = checksum;

	_adjust_buf(tmp_buf, 15, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		//等待总线空闲
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	g_reader_rxcnt = 0;
	reader_recv(10000);	
}	
#endif

/* Start recieve for $time ms */
void reader_recv(u32 time)
{
	g_reader_rxcnt = 0;
	USART2->CR1 |= (1 << 8);    //PE中断使能
	USART2->CR1 |= (1 << 5);    //接收缓冲区非空中断使能	
	delay_ms(time);
	USART2->CR1 &= (~(1 << 8));    //PE中断使能
	USART2->CR1 &= (~(1 << 5));    //接收缓冲区非空中断使�

	//reader_handle();
}

void reader_send_cmd(u8 *cmd, u8 len)
{
	u8 i;
	for (i=0; i<len; i++)
	{
		_send_byte(cmd[i]);
		delay_ms(1);
	}
}

u16 reader_main(void)
{
	while (card_info.present == 0)
	{
		delay_ms(200);
	}

	return 0;
}

