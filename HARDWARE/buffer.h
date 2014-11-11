/*
buffer.h
*/

#ifndef __BUFFER_H
#define __BUFFER_H

#define BUFFER_SIZE (128)
struct circle_buf {
	u8 buf[BUFFER_SIZE];
	u8 head;
	u8 tail;   // tailָ�����һ���ַ��ĺ���һ����λ��
};

void buffer_clear(struct *circle_buf);
u8 buffer_size(struct *circle_buf);
void buffer_push_byte(struct *circle_buf, u8 byte);
u8 buffer_pop_byte(struct *circle_buf);
u8 buffer_pop_nbytes(struct circle_buf* buf, u8 n);
u8 buffer_chk_nbyte(struct circle_buf *buf, u8 n);


#define BUF_HEAD(buf) (buf->buf[buf->head])

#endif  // __BUFFER_H

