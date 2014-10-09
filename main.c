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

/* Newest command recieved */
//u8 g_main_cmd = CONSOLE_CMD_PAUSE;

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
	//motor_init();
	
	delay_ms(500);
	
	/* LEDs initializing */
	//led_init();

	/* Printer initializing (UART3) */
	//printer_init(9600);

	/* Card-reader initializing (UART2) */
	reader_init(9600);

	/* LCD initializing (UART1) */
	console_init(9600);

	/* PC communication initializing (UART4)
	comm_init(9600);
	*/
	
	while (1)
	{
#if 0
		g_main_cmd = console_recv_cmd();
		switch (g_cmd)
		{
		case CONSOLE_CMD_RUNNING:
			channel_main();
			break;
		case CONSOLE_CMD_PAUSE:
			channel_pause();
			break;			
		case CONSOLE_CMD_RESUME:
			channel_resume();
			break;
		default:
			break;
		}
#endif
		//channel_main();
		//printer_main();
		delay_ms(200);
	}	 
}























