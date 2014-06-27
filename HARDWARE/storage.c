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
	u16 tmpreg = 0, freqrange = 0;
  	u16 result = 0x04;
  
  	/* I2C1 Deinit */
	RCC->APB1RSTR |= (1 << 21);   	//复位I2C_1
	RCC->APB1RSTR &= ~(1 << 21);	//复位结束
	RCC->APB1ENR  |= (1 << 21);		//使能I2C_1
	
	/* I2Cx CR2 Configuration */
	tmpreg = I2C1->CR2;
	tmpreg &= 0xFFC0;    // Disable I2C frequence
	freqrange = (u16)(PCLK1_FREQ / 1000000);  //???
	tmpreg |= freqrange;
	I2C1->CR2 = tmpreg;

	/* I2Cx CCR Configuration */
  	/* Disable the selected I2C peripheral to configure TRISE */
  	I2C1->CR1 &= 0xFFFE;

  	/* Configure speed in standard mode, i2c clock speed = 100KHZ */
  	tmpreg = 0;							  
    result = (u16)(PCLK1_FREQ / (I2C1_CLOCK_SPEED << 1));
    if (result < 0x04)      
    	result = 0x04;     
    tmpreg |= result;
		  
    I2C1->TRISE = freqrange + 1;   	
  	I2C1->CCR = tmpreg;	 //???
	I2C1->CR1 |= 0x0001;

	/* I2Cx CR1 Configuration */
    tmpreg = I2C1->CR1;
    tmpreg &= 0xFBF5; 
	tmpreg |= 0x0400;   // Enable ACK
    I2C1->CR1 = tmpreg;

	/* Set I2Cx Own Address1 and acknowledged address */
  	I2C1->OAR1 = 0x4000;

	/* Enable Buffer Interrupt */
	I2C1->CR2 |= 0x0400; 
}	 

/*******************************************************************************
* Function Name  : I2C_GenerateSTART
* Description    : Generates I2Cx communication START condition.
* Input          : - I2Cx: where x can be 1 or 2 to select the I2C peripheral.
*                  - NewState: new state of the I2C START condition generation.
*                    This parameter can be: ENABLE or DISABLE.
* Output         : None
* Return         : None.
*******************************************************************************/
void i2c1_gen_start(bool enable)
{
  if (enable)
    /* Generate a START condition */
    I2C1->CR1 |= 0x0100;
  else
    /* Disable the START condition generation */
    I2C1->CR1 &= 0xFEFF;
}

/*******************************************************************************
* Function Name  : I2C_GenerateSTOP
* Description    : Generates I2Cx communication STOP condition.
* Input          : - I2Cx: where x can be 1 or 2 to select the I2C peripheral.
*                  - NewState: new state of the I2C STOP condition generation.
*                    This parameter can be: ENABLE or DISABLE.
* Output         : None
* Return         : None.
*******************************************************************************/
 void i2c1_gen_stop(bool enable)
{
  if (enable)
    /* Generate a STOP condition */
    I2C1->CR1 |= 0x0200;
  else
    /* Disable the STOP condition generation */
    I2C1->CR1 &= 0xFDFF;
}

/*******************************************************************************
* Function Name  : I2C_AcknowledgeConfig
* Description    : Enables or disables the specified I2C acknowledge feature.
* Input          : - I2Cx: where x can be 1 or 2 to select the I2C peripheral.
*                  - NewState: new state of the I2C Acknowledgement.
*                    This parameter can be: ENABLE or DISABLE.
* Output         : None
* Return         : None.
*******************************************************************************/
 void i2c1_ack_config(bool enable)
{
  if (enable)
    I2C1->CR1 |= 0x0400;
  else
    I2C1->CR1 &= 0xFBFF;
}

/*******************************************************************************
* Function Name  : I2C_SendData
* Description    : Sends a data byte through the I2Cx peripheral.
* Input          : - I2Cx: where x can be 1 or 2 to select the I2C peripheral.
*                  - Data: Byte to be transmitted..
* Output         : None
* Return         : None
*******************************************************************************/
void i2c1_sned_data(u8 Data)
{
  	/* Write in the DR register the data to be sent */
  	I2C1->DR = Data;
}

/*******************************************************************************
* Function Name  : I2C_ReceiveData
* Description    : Returns the most recent received data by the I2Cx peripheral.
* Input          : - I2Cx: where x can be 1 or 2 to select the I2C peripheral.
* Output         : None
* Return         : The value of the received data.
*******************************************************************************/
u8 i2c1_revc_data(void)
{
  /* Return the data in the DR register */
  return (u8)I2C1->DR;
}
