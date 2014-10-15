#ifndef __24CXX_H
#define __24CXX_H
#include "sys.h"

//IO��������
#define SDA_IN()  {GPIOB->CRL &= 0X0FFFFFFF; GPIOB->CRL |= (8 << 7);}
#define SDA_OUT() {GPIOB->CRL &= 0X0FFFFFFF; GPIOB->CRL |= (3 << 7);}

//IO��������	 
#define IIC_SCL    PBout(6) //SCL
#define IIC_SDA    PBout(7) //SDA	 
#define READ_SDA   PBin(7)  //����SDA 


#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
//Mini STM32������ʹ�õ���24c02�����Զ���EE_TYPEΪAT24C02
#define EE_TYPE AT24C02
					  
void storage_init(void);
void storage_read(void);
void storage_write(void);

#endif
















