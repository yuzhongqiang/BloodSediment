/*
buffer.h
*/

#ifndef __BUFFER_H
#define __BUFFER_H

#define BUFFER_SIZE (128)
struct circle_buf {
	u8 buf[BUFFER_SIZE];
	u8 head;
	u8 tail;   // tail指向最后一个字符的后面一个空位置
};

void buffer_clear(struct *circle_buf);
u8 buffer_size(struct *circle_buf);
void buffer_push_byte(struct *circle_buf, u8 byte);
u8   buffer_pop_byte(struct *circle_buf);


#endif  // __BUFFER_H

