/*
* sensor.h
*/

#ifndef __PRINTER_H
#define __PRINTER_H

void printer_init(u32);
void printer_main(void);
u8 print_ch(char ch);
/* str±ØÐëÒÔ\0×Ö·û½áÎ² */
void print_str(char* str);
void printer_test(void);
void printer_do_print(u8 *buf, u16 len);
void printer_graph(u8 idx);

#endif /* __PRINTER_H */

