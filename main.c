#include <stm32f10x_lib.h>
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h" 
#include "key.h"
#include "channel.h"
#include "motor.h"	 	 
#include "printer.h"	 
#include "console.h"	 
#include "reader.h"	 
#include "comm.h"
#include "rtc.h"
#include "storage.h"

/* Newest command recieved */
u8 g_cmd = CONSOLE_STAT_INIT;

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
	
	//channel_init();
	//channel_init_for_debug();
	//motor_init();
	
	delay_ms(500);
	
	/* LEDs initializing */
	//led_init();

	/* Printer initializing (UART3) */
	//printer_init(9600);

	/* Card-reader initializing (UART2) */
	//reader_init(9600);

	/* LCD initializing (UART1) */
	//console_init(9600);

	/* PC communication initializing (UART4)
	comm_init(9600);
	*/
	
	storage_init();
	while (1) {
		
		delay_ms(1);
	}

	while (1)
	{
#if 1
		g_cmd = console_recv_cmd();
		switch (g_cmd)
		{
		case CONSOLE_STAT_RUNNING:
		    channel_main();
			break;
		case CONSOLE_STAT_PAUSE:
			channel_pause();
			break;			
		default:
			break;
		}
#endif

		//printer_main();
		delay_ms(200);
	}	 
}























