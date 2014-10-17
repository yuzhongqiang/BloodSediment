/*
* sensor.h
*/

#ifndef __LCD_H
#define __LCD_H

void console_init(u32);

u8 console_recv_cmd(void);

void console_send_str(u8* str);


#endif /* __LCD_H */
