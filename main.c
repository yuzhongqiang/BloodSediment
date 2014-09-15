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
#include "rtc.h"	 

/* Scanning ECR now */
extern u8 g_is_scanning;

int main(void)
{			
	/* Set vector table */

	/* System clock initiliaing */
	sys_init();
	delay_init(72);
	
	/* Real time init */
	rtc_init();
	rtc_set(1970,1,1,0,0,0);
	
	/* keys initializing
	key_init();
	*/
	
	channel_init();
	motor_init();
	
	delay_ms(3000);
	
	/* LEDs initializing */
	//led_init();

	/* Printer initializing (UART3) */
	printer_init(9600);

	/* Card-reader initializing (UART2)
	reader_init(9600);
	*/

	/* LCD initializing (UART1)
	lcd_init(9600);
	*/

	/* PC communication initializing (UART4)
	comm_init(9600);
	*/
	
	while (1)
	{
		channel_main();
			
		do_print();

		delay_ms(500);
	}	 
}























