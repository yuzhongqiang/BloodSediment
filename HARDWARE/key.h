#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define KEY0 PAin(13)   //PA13
#define KEY1 PAin(15)	//PA15 
#define KEY2 PAin(0)	//PA0  WK_UP
	 
void key_init(void);//IO��ʼ��
u8 key_scan(void);  //����ɨ�躯��					    

#endif  // __KEY_H

