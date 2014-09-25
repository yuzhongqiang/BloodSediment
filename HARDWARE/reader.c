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

/* IC Reader uses USART2 */

/* ����2�жϷ������
   ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
*/   	
u8 g_reader_rxbuf[64];     //���ջ���,���64���ֽ�.
u8 g_reader_rxcnt = 0;
  
void USART2_IRQHandler(void)
{
	u8 res;	    
	if (USART2->SR & (1<<5))//���յ�����
	{	 
		res = USART2->DR; 
		g_reader_rxbuf[g_reader_rxcnt] = res;
		g_reader_rxcnt++;
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
	//USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	//USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	    	
	nvic_init(3, 3, USART2_IRQChannel, 2);//��2��������ȼ� 
}

static u8 _send_byte(u8 ch)
{
	while ((USART3->SR & 0x40) == 0)	//�ȴ����߿���
		;

	USART3->DR = ch;      
	return ch;
}

static u8 reader_checksum(u8* data, u8 offset, u8 len)
{
	u8 temp = 0, i;
	
	for (i = offset; i < len + offset; i++)
	{
		temp ^= data[i];
	}
	return temp; 
}

/*
	���ݴ�Ÿ�ʽ����:
	+---------------------------------------------------------------------------------------
	| ����ͷ(1 byte) 	�����(1 byte)	������(1 byte)	����(n byte)	У��(1 byte) |
	+---------------------------------------------------------------------------------------
	| 7F						2~7E					xx					��				CRC			    |	
	+---------------------------------------------------------------------------------------

	���ݴ��λ��:
	Block4: �ܳ�ֵ��Ŀ
	Block7: Block4~7����������key�ʹ�ȡ������:
			keyA:             0xff 0xff 0xff 0xff 0xff 0xff
			Control Code: 0xff 0x03 0x80 0x00  [Block0~2:rw  Block4:keyA(w) CC(rw) keyB(rw) -- need keyA|keyB]
			keyB:             0xff 0xff 0xff 0xff 0xff 0xff
*/

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

/* �ı�BlockX��keyA|CC|keyB */
void reader_change_cc(u8 block)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[60];
	u8 i;
	
	u8 tmp_buf[27] = {0x7f, 0x19/*len*/, 0x15/*cmd*/, 0x04/*block#*/,
				    	/*keyB*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*data-keyA*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*data-CC*/0xff, 0x03, 0x80, 0x00,
						/*data-keyB*/0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						/*checksum*/};
	checksum = reader_checksum(tmp_buf, 2, 24);
	tmp_buf[14] = checksum;

	_adjust_buf(tmp_buf, 15, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		_send_byte(buf[i]);
		delay_us(100);   // ???
	}
}

/*  ��Ƭһ����ֵ
0x12	һ����ֵ	���ַ��1��
��֤����ԿB��6��
��ֵ��4��	״̬��1��
�����ͣ�2��
���ţ�4��
��4��
*/
void reader_store_value(u16 value)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[32];
	u8 i;
	
	u8 tmp_buf[15] = {0x7f, 0x0d/*0x0e?*/, 0x12, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,};
	*((u32*)(tmp_buf+10)) = value;
	checksum = reader_checksum(tmp_buf, 2, 12/*13?*/);
	tmp_buf[14] = checksum;

	_adjust_buf(tmp_buf, 15, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		_send_byte(buf[i]);
		delay_us(100);   // ???
	}
}

/* һ����ֵ
0x13	һ���ۿ�	���ַ��1��
��֤����ԿA��6��
�ۿ��4��	״̬��1��
�����ͣ�2��
���ţ�4��
��4��
*/
void reader_load_value(void)
{
	u8 checksum = 0;	
	u8 data_len = 0;
	u8 buf[32];
	u8 i;
	
	u8 tmp_buf[15] = {0x7f, 0x03, 0x06, 0x04,};
	checksum = reader_checksum(tmp_buf, 1, 3);
	tmp_buf[4] = checksum;

	_adjust_buf(tmp_buf, 5, buf, &data_len);
	for (i=0; i<data_len; i++)
	{
		_send_byte(buf[i]);
		delay_us(100);   // ???
	}	
}	

void reader_recv(void)
{
	g_reader_rxcnt = 0;
	USART2->CR1 |= (1 << 8);    //PE�ж�ʹ��
	USART2->CR1 |= (1 << 5);    //���ջ������ǿ��ж�ʹ��	
	delay_ms(3000);
	USART2->CR1 &= (~(1 << 8));    //PE�ж�ʹ��
	USART2->CR1 &= (~(1 << 5));    //���ջ������ǿ��ж�ʹ�

	//reader_handle();
}

void reader_send_cmd(u8 *cmd, u8 len)
{
	u8 i;
	for (i=0; i<len; i++)
		_send_byte(cmd[i]);
}


