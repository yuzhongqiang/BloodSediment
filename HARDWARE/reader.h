/*
* reader.h
*/

#ifndef __READER_H
#define __READER_H

struct _card_info {
	u8 	present;
	u8  cardinfo[20];
	u16 value;   // 卡中存储的数据
	u8 status;
};

void reader_init(u32);
void reader_recv(u32 time);
void reader_handle(void);
#if 0
void reader_get_cardinfo(void);
#endif
void reader_close_card(void);
u16 reader_read_block(u8 blk);
void reader_write_block(u8 blk, u16 value);
u16 reader_main(void);


#endif /* __READER_H */

