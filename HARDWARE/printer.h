/*
* sensor.h
*/

#ifndef __PRINTER_H
#define __PRINTER_H

void printer_init(u32);




u8 print_ch(u8 ch);

/* str±ØÐëÒÔ\0×Ö·û½áÎ² */
void print_str(u8* str);




#endif /* __PRINTER_H */
