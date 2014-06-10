#include <stm32f10x_lib.h>
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key.h"
#include "sensor.h"	 	 
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
	sensor_init();
	
	/* LEDs initializing */
	led_init();

	/* Printer initializing (UART1) */
	printer_init();

	/* Card-reader initializing (UART2) */
	reader_init();

	/* LCD initializing (UART3) */
	lcd_init();

	/* PC communication initializing (UART4) */
	comm_init();

	while(1)
	{
		;
	}	 
}

























