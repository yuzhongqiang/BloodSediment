#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

//LED�˿ڶ���
#define LED0 PAout(8)// PA8
#define LED1 PDout(2)// PD2	

void led_init(void);//��ʼ��	
void led_run(void); //���Դ��룺LED����Ƴ���
	 				    
#endif  // __LED_H

