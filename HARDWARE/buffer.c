/*
buffer.c
*/

#include "buffer.h"

void buffer_clear(struct circle_buf* buf)
{
	buf->head = 0;
	buf->tail = 0;
}

u8 buffer_size(struct circle_buf* buf)
{
	return ((buf->tail + BUFFER_SIZE - buf->head) % BUFFER_SIZE);
}

void buffer_push_byte(struct circle_buf* buf, u8 byte)
{
	/* 注意此处没有处理溢出*/
	buf->buf[buf->tail] = byte;
	buf->tail = ((buf->tail + 1) % BUFFER_SIZE);
}

u8 buffer_pop_byte(struct circle_buf* buf)
{
	u8 tmp;

	if (buf->tail == buf->head)
		return 0;
	tmp = buf->buf[buf->head];
	buf->head = ((buf->head + 1)  % BUFFER_SIZE);
	return tmp;
}

u8 buffer_pop_nbytes(struct circle_buf* buf, u8 n)
{
	u8 size = buffer_size(buf);
	size = ((size > n) ? n : size);
	buf->head += size;
	return 0;
}

u8 buffer_chk_nbyte(struct circle_buf *buf, u8 n)
{
	return (buf->buf[buf->head+n]);
}

