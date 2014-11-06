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
	buf->tail = ((buf->tail + 1) % BUFFER_SIZE);
	buf->buf[buf->tail] = byte;
}

u8   buffer_pop_byte(struct circle_buf* buf)
{
	u8 tmp = buf->buf[buf->head];
	buf->head = ((buf->head + 1)  % BUFFER_SIZE);
	return tmp;
}

