/*
* sensor.h
*/

#ifndef __PRINTER_H
#define __PRINTER_H





void printer_init(u32);




u8 print_ch(char ch);

/* str±ØÐëÒÔ\0×Ö·û½áÎ² */
void print_str(char* str);

void do_print(void);

void printer_test(void);

#endif /* __PRINTER_H */
