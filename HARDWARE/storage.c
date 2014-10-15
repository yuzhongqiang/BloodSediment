/*
* Storage.c
*
*/

#include <stm32f10x_lib.h>	
#include <stm32f10x_i2c.h>
#include <stm32f10x_rcc.h>
#include "sys.h"
#include "delay.h"
#include "storage.h"

/* Storage uses I2C1 */
#if 0

void I2C_Init(I2C_TypeDef* I2Cx, I2C_InitTypeDef* I2C_InitStruct)
{
  u16 tmpreg = 0, freqrange = 0;
  u16 result = 0x04;
  u32 pclk1 = 8000000;
  RCC_ClocksTypeDef  rcc_clocks;

  /* Check the parameters */
  assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  assert_param(IS_I2C_MODE(I2C_InitStruct->I2C_Mode));
  assert_param(IS_I2C_DUTY_CYCLE(I2C_InitStruct->I2C_DutyCycle));
  assert_param(IS_I2C_OWN_ADDRESS1(I2C_InitStruct->I2C_OwnAddress1));
  assert_param(IS_I2C_ACK_STATE(I2C_InitStruct->I2C_Ack));
  assert_param(IS_I2C_ACKNOWLEDGE_ADDRESS(I2C_InitStruct->I2C_AcknowledgedAddress));
  assert_param(IS_I2C_CLOCK_SPEED(I2C_InitStruct->I2C_ClockSpeed));

/*---------------------------- I2Cx CR2 Configuration ------------------------*/
  /* Get the I2Cx CR2 value */
  tmpreg = I2Cx->CR2;
  /* Clear frequency FREQ[5:0] bits */
  tmpreg &= CR2_FREQ_Reset;
  /* Get pclk1 frequency value */
  RCC_GetClocksFreq(&rcc_clocks);
  pclk1 = rcc_clocks.PCLK1_Frequency;
  /* Set frequency bits depending on pclk1 value */
  freqrange = (u16)(pclk1 / 1000000);
  tmpreg |= freqrange;
  /* Write to I2Cx CR2 */
  I2Cx->CR2 = tmpreg;

/*---------------------------- I2Cx CCR Configuration ------------------------*/
  /* Disable the selected I2C peripheral to configure TRISE */
  I2Cx->CR1 &= CR1_PE_Reset;

  /* Reset tmpreg value */
  /* Clear F/S, DUTY and CCR[11:0] bits */
  tmpreg = 0;

  /* Configure speed in standard mode */
  if (I2C_InitStruct->I2C_ClockSpeed <= 100000)
  {
    /* Standard mode speed calculate */
    result = (u16)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed << 1));
    /* Test if CCR value is under 0x4*/
    if (result < 0x04)
    {
      /* Set minimum allowed value */
      result = 0x04;  
    }
    /* Set speed value for standard mode */
    tmpreg |= result;	  
    /* Set Maximum Rise Time for standard mode */
    I2Cx->TRISE = freqrange + 1; 
  }
  /* Configure speed in fast mode */
  else /*(I2C_InitStruct->I2C_ClockSpeed <= 400000)*/
  {
    if (I2C_InitStruct->I2C_DutyCycle == I2C_DutyCycle_2)
    {
      /* Fast mode speed calculate: Tlow/Thigh = 2 */
      result = (u16)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed * 3));
    }
    else /*I2C_InitStruct->I2C_DutyCycle == I2C_DutyCycle_16_9*/
    {
      /* Fast mode speed calculate: Tlow/Thigh = 16/9 */
      result = (u16)(pclk1 / (I2C_InitStruct->I2C_ClockSpeed * 25));
      /* Set DUTY bit */
      result |= I2C_DutyCycle_16_9;
    }
    /* Test if CCR value is under 0x1*/
    if ((result & CCR_CCR_Set) == 0)
    {
      /* Set minimum allowed value */
      result |= (u16)0x0001;  
    }
    /* Set speed value and set F/S bit for fast mode */
    tmpreg |= result | CCR_FS_Set;
    /* Set Maximum Rise Time for fast mode */
    I2Cx->TRISE = (u16)(((freqrange * 300) / 1000) + 1);  
  }
  /* Write to I2Cx CCR */
  I2Cx->CCR = tmpreg;

  /* Enable the selected I2C peripheral */
  I2Cx->CR1 |= CR1_PE_Set;

/*---------------------------- I2Cx CR1 Configuration ------------------------*/
  /* Get the I2Cx CR1 value */
  tmpreg = I2Cx->CR1;
  /* Clear ACK, SMBTYPE and  SMBUS bits */
  tmpreg &= CR1_CLEAR_Mask;
  /* Configure I2Cx: mode and acknowledgement */
  /* Set SMBTYPE and SMBUS bits according to I2C_Mode value */
  /* Set ACK bit according to I2C_Ack value */
  tmpreg |= (u16)((u32)I2C_InitStruct->I2C_Mode | I2C_InitStruct->I2C_Ack);
  /* Write to I2Cx CR1 */
  I2Cx->CR1 = tmpreg;

/*---------------------------- I2Cx OAR1 Configuration -----------------------*/
  /* Set I2Cx Own Address1 and acknowledged address */
  I2Cx->OAR1 = (I2C_InitStruct->I2C_AcknowledgedAddress | I2C_InitStruct->I2C_OwnAddress1);
}
#endif

void I2C_StructInit(I2C_InitTypeDef* I2C_InitStruct)
{
/*---------------- Reset I2C init structure parameters values ----------------*/
  /* Initialize the I2C_Mode member */
  I2C_InitStruct->I2C_Mode = I2C_Mode_I2C;

  /* Initialize the I2C_DutyCycle member */
  I2C_InitStruct->I2C_DutyCycle = I2C_DutyCycle_2;

  /* Initialize the I2C_OwnAddress1 member */
  I2C_InitStruct->I2C_OwnAddress1 = 0;

  /* Initialize the I2C_Ack member */
  I2C_InitStruct->I2C_Ack = I2C_Ack_Disable;

  /* Initialize the I2C_AcknowledgedAddress member */
  I2C_InitStruct->I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

  /* initialize the I2C_ClockSpeed member */
  I2C_InitStruct->I2C_ClockSpeed = 5000;
}

void I2C_GenerateSTART(I2C_TypeDef* I2Cx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Generate a START condition */
    I2Cx->CR1 |= CR1_START_Set;
  }
  else
  {
    /* Disable the START condition generation */
    I2Cx->CR1 &= CR1_START_Reset;
  }
}

void I2C_Send7bitAddress(I2C_TypeDef* I2Cx, u8 Address, u8 I2C_Direction)
{
  /* Check the parameters */
  assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  assert_param(IS_I2C_DIRECTION(I2C_Direction));

  /* Test on the direction to set/reset the read/write bit */
  if (I2C_Direction != I2C_Direction_Transmitter)
  {
    /* Set the address bit0 for read */
    Address |= OAR1_ADD0_Set;
  }
  else
  {
    /* Reset the address bit0 for write */
    Address &= OAR1_ADD0_Reset;
  }
  /* Send the address */
  I2Cx->DR = Address;
}	
void I2C_GenerateSTOP(I2C_TypeDef* I2Cx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Generate a STOP condition */
    I2Cx->CR1 |= CR1_STOP_Set;
  }
  else
  {
    /* Disable the STOP condition generation */
    I2Cx->CR1 &= CR1_STOP_Reset;
  }
}

#if 0
void storage_init(void)
{
	u16 tmpreg = 0, freqrange = 0;
  	u16 result = 0x04;
  
  	/* I2C1 Deinit */
	RCC->APB1RSTR |= (1 << 21);   	//复位I2C_1
	delay_us(100);
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
	//////I2C1->CR2 |= 0x0400; 
}	 

void i2c1_gen_start(bool enable)
{
  if (enable)
    /* Generate a START condition */
    I2C1->CR1 |= 0x0100;
  else
    /* Disable the START condition generation */
    I2C1->CR1 &= 0xFEFF;
}

 void i2c1_gen_stop(bool enable)
{
  if (enable)
    /* Generate a STOP condition */
    I2C1->CR1 |= 0x0200;
  else
    /* Disable the STOP condition generation */
    I2C1->CR1 &= 0xFDFF;
}

 void i2c1_ack_config(bool enable)
{
  if (enable)
    I2C1->CR1 |= 0x0400;
  else
    I2C1->CR1 &= 0xFBFF;
}

void i2c1_send_data(u8 Data)
{
  	/* Write in the DR register the data to be sent */
  	I2C1->DR = Data;
}

u8 i2c1_revc_data(void)
{
  /* Return the data in the DR register */
  return (u8)I2C1->DR;
}

void storage_send_data(u8 data)
{
	u16 temp;

	i2c1_gen_start(1);
	temp = I2C1->SR1;
	I2C1->DR = 0x30;
	while (!(temp &	0x0001))
		temp = I2C1->SR1;

 	i2c1_send_data(data);
	i2c1_gen_stop(1);
}
#endif

ErrorStatus I2C_CheckEvent(I2C_TypeDef* I2Cx, u32 I2C_EVENT)
{
  u32 lastevent = 0;
  u32 flag1 = 0, flag2 = 0;
  ErrorStatus status = ERROR;

  /* Check the parameters */
  assert_param(IS_I2C_ALL_PERIPH(I2Cx));
  assert_param(IS_I2C_EVENT(I2C_EVENT));

  /* Read the I2Cx status register */
  flag1 = I2Cx->SR1;
  flag2 = I2Cx->SR2;
  flag2 = flag2 << 16;

  /* Get the last event value from I2C status register */
  lastevent = (flag1 | flag2) & FLAG_Mask;

  /* Check whether the last event is equal to I2C_EVENT */
  if (lastevent == I2C_EVENT )
  {
    /* SUCCESS: last event is equal to I2C_EVENT */
    status = SUCCESS;
  }
  else
  {
    /* ERROR: last event is different from I2C_EVENT */
    status = ERROR;
  }

  /* Return status */
  return status;
}

void storage_init(void)
{
   I2C_InitTypeDef init_type;

   I2C_StructInit(&init_type);
   init_type.I2C_Ack = I2C_Ack_Enable;
   init_type.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
   init_type.I2C_ClockSpeed = 100000;
   I2C_Init(I2C1, &init_type);

   I2C_GenerateSTART(I2C1, ENABLE);
   while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
   	;
   I2C_Send7bitAddress(I2C1, 0x30, I2C_Direction_Transmitter);
   while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
   	;

}

void storage_send_data(u8 data)
{

}














