#include <stm32f10x_lib.h>
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key.h"
#include "detector.h"	 	 
#include "printer.h"	 
#include "lcd.h"	 
#include "reader.h"	 
#include "comm.h"	 

int main(void)
{			
	/* Set vector table */


	/* System clock initiliaing */
	sys_init();
	delay_init(72);	     //—” ±≥ı ºªØ 

	/* keys initializing */
	key_init();

	/* Blood sedimentation sensors initializing */
	detector_init();
	
	/* LEDs initializing */
	led_init();

	/* Printer initializing (UART1) */
	printer_init(9600);

	/* Card-reader initializing (UART2) */
	reader_init(9600);

	/* LCD initializing (UART3) */
	lcd_init(9600);

	/* PC communication initializing (UART4) */
	comm_init(9600);

	while(1)
	{
		;
	}	 
}

























