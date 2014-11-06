#include "storage.h" 
#include "delay.h" 

//³õÊ¼»¯IIC
void IIC_Init(void)
{					     
 	//RCC->APB2ENR| = 1<<4;//ÏÈÊ¹ÄÜÍâÉèIO PORTCÊ±ÖÓ 							 
	GPIOB->CRL &= 0x00FFFFFF;//PB6-SCL; PB7-SDA ÍÆÍìÊä³ö
	GPIOB->CRL |= 0x33000000;	   
	GPIOB->ODR |= (3 << 6);     //PB6 PB7Êä³ö¸ß
}

//²úÉúIICÆğÊ¼ĞÅºÅ
void IIC_Start(void)
{
	SDA_OUT();     //sdaÏßÊä³ö
	IIC_SDA = 1;	  	  
	IIC_SCL = 1;
	delay_us(4);
 	IIC_SDA = 0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL = 0;//Ç¯×¡I2C×ÜÏß£¬×¼±¸·¢ËÍ»ò½ÓÊÕÊı¾İ 
}

//²úÉúIICÍ£Ö¹ĞÅºÅ
void IIC_Stop(void)
{
	SDA_OUT();//sdaÏßÊä³ö
	IIC_SCL = 0;
	IIC_SDA = 0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL = 1; 
	IIC_SDA = 1;//·¢ËÍI2C×ÜÏß½áÊøĞÅºÅ
	delay_us(4);							   	
}

//µÈ´ıÓ¦´ğĞÅºÅµ½À´
//·µ»ØÖµ£º1£¬½ÓÊÕÓ¦´ğÊ§°Ü
//        0£¬½ÓÊÕÓ¦´ğ³É¹¦
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime = 0;

	SDA_IN();      //SDAÉèÖÃÎªÊäÈë  
	IIC_SDA = 1;
	delay_us(1);	   
	IIC_SCL = 1;
	delay_us(1);	 
	while (READ_SDA)
	{
		ucErrTime++;
		if (ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL = 0;//Ê±ÖÓÊä³ö0 

	return 0;  
} 

//²úÉúACKÓ¦´ğ
void IIC_Ack(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 0;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}

//²»²úÉúACKÓ¦´ğ		    
void IIC_NAck(void)
{
	IIC_SCL = 0;
	SDA_OUT();
	IIC_SDA = 1;
	delay_us(2);
	IIC_SCL = 1;
	delay_us(2);
	IIC_SCL = 0;
}

//IIC·¢ËÍÒ»¸ö×Ö½Ú
//·µ»Ø´Ó»úÓĞÎŞÓ¦´ğ
//1£¬ÓĞÓ¦´ğ
//0£¬ÎŞÓ¦´ğ			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL = 0;//À­µÍÊ±ÖÓ¿ªÊ¼Êı¾İ´«Êä
    for (t = 0; t < 8; t++)
    {              
        IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1; 	  
		delay_us(2);   //¶ÔTEA5767ÕâÈı¸öÑÓÊ±¶¼ÊÇ±ØĞëµÄ
		IIC_SCL = 1;
		delay_us(2); 
		IIC_SCL = 0;	
		delay_us(2);
    }	 
}

//¶Á1¸ö×Ö½Ú£¬ack=1Ê±£¬·¢ËÍACK£¬ack=0£¬·¢ËÍnACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive = 0;
	SDA_IN();//SDAÉèÖÃÎªÊäÈë
    for (i = 0; i < 8; i++ )
	{
        IIC_SCL = 0; 
        delay_us(2);
		IIC_SCL = 1;
        receive <<= 1;
        if (READ_SDA)
			receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//·¢ËÍnACK
    else
        IIC_Ack(); //·¢ËÍACK  
 
    return receive;
}

/************************************************************
                                   AT24CXX Routines
************************************************************/

//³õÊ¼»¯IIC½Ó¿Ú
void AT24CXX_Init(void)
{
	IIC_Init();
}

//ÔÚAT24CXXÖ¸¶¨µØÖ·¶Á³öÒ»¸öÊı¾İ
//ReadAddr:¿ªÊ¼¶ÁÊıµÄµØÖ·  
//·µ»ØÖµ  :¶Áµ½µÄÊı¾İ
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp = 0;		  	    																 
    IIC_Start();  
	if (EE_TYPE>AT24C16) {
		IIC_Send_Byte(0XA0);	   //·¢ËÍĞ´ÃüÁî
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//·¢ËÍ¸ßµØÖ·	    
	}
	else
		IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //·¢ËÍÆ÷¼şµØÖ·0XA0,Ğ´Êı¾İ 	   

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr % 256);   //·¢ËÍµÍµØÖ·
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //½øÈë½ÓÊÕÄ£Ê½			   
	IIC_Wait_Ack();	 
    temp = IIC_Read_Byte(0);		   
    IIC_Stop();//²úÉúÒ»¸öÍ£Ö¹Ìõ¼ş	    
	return temp;
}

//ÔÚAT24CXXÖ¸¶¨µØÖ·Ğ´ÈëÒ»¸öÊı¾İ
//WriteAddr  :Ğ´ÈëÊı¾İµÄÄ¿µÄµØÖ·    
//DataToWrite:ÒªĞ´ÈëµÄÊı¾İ
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();  
	if (EE_TYPE > AT24C16)	{
		IIC_Send_Byte(0XA0);	    //·¢ËÍĞ´ÃüÁî
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//·¢ËÍ¸ßµØÖ·	  
	}else
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //·¢ËÍÆ÷¼şµØÖ·0XA0,Ğ´Êı¾İ 	 
		
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr % 256);   //·¢ËÍµÍµØÖ·
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //·¢ËÍ×Ö½Ú							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();//²úÉúÒ»¸öÍ£Ö¹Ìõ¼ş 
	delay_ms(10);	 
}

//ÔÚAT24CXXÀïÃæµÄÖ¸¶¨µØÖ·¿ªÊ¼Ğ´Èë³¤¶ÈÎªLenµÄÊı¾İ
//¸Ãº¯ÊıÓÃÓÚĞ´Èë16bit»òÕß32bitµÄÊı¾İ.
//WriteAddr  :¿ªÊ¼Ğ´ÈëµÄµØÖ·  
//DataToWrite:Êı¾İÊı×éÊ×µØÖ·
//Len        :ÒªĞ´ÈëÊı¾İµÄ³¤¶È2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	
	for (t = 0; t < Len; t++) {
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite >> (8*t)) & 0xff);
	}												    
}

//ÔÚAT24CXXÀïÃæµÄÖ¸¶¨µØÖ·¿ªÊ¼¶Á³ö³¤¶ÈÎªLenµÄÊı¾İ
//¸Ãº¯ÊıÓÃÓÚ¶Á³ö16bit»òÕß32bitµÄÊı¾İ.
//ReadAddr   :¿ªÊ¼¶Á³öµÄµØÖ· 
//·µ»ØÖµ     :Êı¾İ
//Len        :Òª¶Á³öÊı¾İµÄ³¤¶È2,4
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{  	
	u8 t;
	u32 temp = 0;
	for (t = 0;t < Len; t++) {
		temp <<= 8;
		temp += AT24CXX_ReadOneByte(ReadAddr + Len - t - 1); 	 				   
	}
	return temp;												    
}

//¼ì²éAT24CXXÊÇ·ñÕı³£
//ÕâÀïÓÃÁË24XXµÄ×îºóÒ»¸öµØÖ·(255)À´´æ´¢±êÖ¾×Ö.
//Èç¹ûÓÃÆäËû24CÏµÁĞ,Õâ¸öµØÖ·ÒªĞŞ¸Ä
//·µ»Ø1:¼ì²âÊ§°Ü
//·µ»Ø0:¼ì²â³É¹¦
u8 AT24CXX_Check(void)
{
	u8 temp;
	
	temp = AT24CXX_ReadOneByte(255);//±ÜÃâÃ¿´Î¿ª»ú¶¼Ğ´AT24CXX			   
	if(temp == 0X55)
		return 0;		   
	else {		//ÅÅ³ıµÚÒ»´Î³õÊ¼»¯µÄÇé¿ö
		AT24CXX_WriteOneByte(255, 0X55);
	    temp = AT24CXX_ReadOneByte(255);	  
		if(temp == 0X55)
			return 0;
	}
	return 1;											  
}

//ÔÚAT24CXXÀïÃæµÄÖ¸¶¨µØÖ·¿ªÊ¼¶Á³öÖ¸¶¨¸öÊıµÄÊı¾İ
//ReadAddr :¿ªÊ¼¶Á³öµÄµØÖ· ¶Ô24c02Îª0~255
//pBuffer  :Êı¾İÊı×éÊ×µØÖ·
//NumToRead:Òª¶Á³öÊı¾İµÄ¸öÊı
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while (NumToRead)
	{
		*pBuffer++ = AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}

//ÔÚAT24CXXÀïÃæµÄÖ¸¶¨µØÖ·¿ªÊ¼Ğ´ÈëÖ¸¶¨¸öÊıµÄÊı¾İ
//WriteAddr :¿ªÊ¼Ğ´ÈëµÄµØÖ· ¶Ô24c02Îª0~255
//pBuffer   :Êı¾İÊı×éÊ×µØÖ·
//NumToWrite:ÒªĞ´ÈëÊı¾İµÄ¸öÊı
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while (NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

/************************************************************
                                     Storage Routines
************************************************************/

/* ²éÑ¯Ê£Óà´ÎÊı
	Ê£Óà´ÎÊı´æ·ÅÔÚeepromµÄµÚ10£,11×Ö½Ú
	10×Ö½ÚÊÇ¸ßÎ»£¬11×Ö½ÚÊÇµÍÎ»  
*/
void storage_init(void)
{
 	AT24CXX_Init();
}

u16 storage_query(void)
{
	return AT24CXX_ReadLenByte(10, 4);
}

void storage_save(u32 value)
{
	AT24CXX_WriteLenByte(10, value, 4);
}

u16 storage_dec(void)
{
	u16 remain;
	
	remain = storage_query();
	storage_save(remain-1);
	return 0;
} 

void storage_add(u32 value)
{
	u16 remain;
	
	remain = storage_query();
	storage_save(remain + value);	
}	   
