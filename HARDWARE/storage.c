/*
* Storage.c
*
*/

#include <stm32f10x_lib.h>	
#include "sys.h"
#include "storage.h"

/* Storage uses I2C1 */
void storage_init(void)
{
	 u32 baud;
	 baud = 9600;
	 GPIOA->CRH = baud;
}	 
