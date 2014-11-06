#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

//LED端口定义
#define LED0 PAout(8)// PA8
#define LED1 PDout(2)// PD2	

void led_init(void);//初始化	
void led_run(void); //测试代码：LED跑马灯程序
	 				    
#endif  // __LED_H

