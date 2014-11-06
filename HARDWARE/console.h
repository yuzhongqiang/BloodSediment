/*
* console.h
*/

#ifndef __CONSOLE_H
#define __CONSOLE_H

void console_init(u32);
u8 console_recv_cmd(void);
void console_send_str(u8* str);

#endif /* __CONSOLE_H */

