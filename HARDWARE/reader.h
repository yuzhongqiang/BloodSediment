/*
* sensor.h
*/

#ifndef __READER_H
#define __READER_H

struct _card_info {
	u8 	present;
	u8  cardno[4];
	u8  type[2];
	u16 value;
	u8 status;
};

void reader_init(u32);
void reader_recv(u32 time);
void reader_handle(void);
u32 reader_read_cardinfo(void);
u16 reader_main(void);


#endif /* __READER_H */
