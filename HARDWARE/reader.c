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

/* ��ų�ֵ���ݵĿ�� */
#define VALUE_BLOCK (0x04)

/* ����2�жϷ������
   ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
*/   	
u8 g_reader_rxbuf[64];     //���ջ���,���64���ֽ�.
u8 g_reader_rxcnt = 0;

#if 0
void _reader_enable_intr(void)
{
	g_reader_rxcnt = 0;
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ�
}

void _reader_disable_intr(void)
{
	USART2->CR1 &= ~(1 << 8);    //PE�ж�ʹ��
	USART2->CR1 &= ~(1 << 5);    //���ջ������ǿ��ж�ʹ�
#endif

void USART2_IRQHandler(void)
{
	u8 res;	
	
	if (USART2->SR & (1<<5))//���յ�����
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
	
			case 0x02:   /* һ�������д��*/
				if (g_reader_rxcnt > 6)     /* ����*/
					card_info.value = ((g_reader_rxbuf[4] << 8) + g_reader_rxbuf[5]);
				else     /* д��*/
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
		   
	RCC->APB2ENR |= 1 << 2;   //ʹ��PORTA��ʱ��  OK
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

	//ʹ�ܽ����ж�
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART2_IRQChannel, 2);//��2��������ȼ� 
}

static u8 _send_byte(u8 ch)
{
	while ((USART2->SR & 0x40) == 0)	//�ȴ����߿���
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
	���ݴ�Ÿ�ʽ����:
	+---------------------------------------------------------------------------------------
	| ����ͷ(1 byte) 	�����(1 byte)	������(1 byte)	����(n byte)	У��(1 byte) |
	+---------------------------------------------------------------------------------------
	| 7F				2~7E				xx				��				CRC			    |	
	+---------------------------------------------------------------------------------------

	���ݴ��λ��:
	Block4: �ܳ�ֵ��Ŀ
	Block7: Block4~7����������key�ʹ�ȡ������:
			keyA:             0xff 0xff 0xff 0xff 0xff 0xff
			Control Code: 0xff 0x07 0x80 0x00  [Block0~2:rw  Block4:keyA(w) CC(rw) keyB(rw) -- need keyA|keyB]
			keyB:             0xff 0xff 0xff 0xff 0xff 0xff
*/

/* �ı�BlockX��keyA|CC|keyB */
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
		//�ȴ����߿���
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	//g_reader_rxcnt = 0;
	//reader_recv(4000);
	
}

/*  ��Ƭһ����ֵ
      command code: 0x12
      block address:   1byte
	��֤����ԿB��6bytes��
	��ֵ��4bytes�

	����ֵ:
		״̬��1��
		�����ͣ�2��
		���ţ�4��
		��4��
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
		//�ȴ����߿���
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	g_reader_rxcnt = 0;
	reader_recv(4000);
}

/* һ������
      command code: 0x10

	����ֵ:
		״̬��1��
		�����ͣ�2��
		���ţ�4��
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
		//�ȴ����߿���
		while ((USART2->SR & 0x40) == 0)
			;
		USART2->DR = buf[i];   
	}

	//g_reader_rxcnt = 0;
	//reader_recv(4000);	
}


/* һ������
      command code: 0x14
      block address:   1byte
	��֤����ԿA��6bytes��

	����ֵ:
		״̬��1��
		�����ͣ�2��
		���ţ�4��
		����(16)
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
		//�ȴ����߿���
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
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	
	delay_ms(time);
	USART2->CR1 &= (~(1 << 8));    //PE�ж�ʹ��
	USART2->CR1 &= (~(1 << 5));    //���ջ������ǿ��ж�ʹ�

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

