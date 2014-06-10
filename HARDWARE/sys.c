#include <stm32f10x_lib.h>		 
#include "sys.h"

//设置向量表偏移地址
//NVIC_VectTab:基址
//Offset:偏移量
static void MY_NVIC_SetVectorTable(u32 NVIC_VectTab, u32 Offset)	 
{   
	//设置NVIC的向量表偏移寄存器
	SCB->VTOR = (NVIC_VectTab | (Offset & (u32)0x1FFFFF80));
}

//设置NVIC分组
//NVIC_Group:NVIC分组 0~4 总共5组 
static void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group)	 
{ 
	u32 temp,temp1;	  

	temp1 = (~NVIC_Group) & 0x07;//取后三位
	temp1 <<= 8;
	temp = SCB->AIRCR;  //读取先前的设置
	temp &= 0X0000F8FF; //清空先前分组
	temp |= 0X05FA0000; //写入钥匙
	temp |= temp1;	   
	SCB->AIRCR = temp;  //设置分组	    	  				   
}

//不能在这里执行所有外设复位!否则至少引起串口不工作.		    
//把所有时钟寄存器复位
static void MYRCC_DeInit(void)
{										  					   
	RCC->APB1RSTR = 0x00000000;//复位结束			 
	RCC->APB2RSTR = 0x00000000; 
	  
  	RCC->AHBENR = 0x00000014;  //睡眠模式闪存和SRAM时钟使能.其他关闭.	  
  	RCC->APB2ENR = 0x00000000; //外设时钟关闭.			   
  	RCC->APB1ENR = 0x00000000;   
	RCC->CR |= 0x00000001;     //使能内部高速时钟HSION	 															 
	RCC->CFGR &= 0xF8FF0000;   //复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR &= 0xFEF6FFFF;     //复位HSEON,CSSON,PLLON
	RCC->CR &= 0xFFFBFFFF;     //复位HSEBYP	   	  
	RCC->CFGR &= 0xFF80FFFF;   //复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR = 0x00000000;     //关闭所有中断

	//配置向量表				  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}

static void pll_clock_init(u32 pll_freq)
{
	RCC->CR |= 0x00010000;            //使能HSEON,启动HSE晶体振荡器
	while (!(RCC-> CR & 0x00020000))  //等待外部时钟就绪
		;				  

	RCC->CFGR = 0X00000400; 		//APB1_DIV=2, PCLK1=pll_freq/2=36MHz;
									//PCLK2=pll_freq=72MHZ;
									//HCLK=pll_freq=72MHz
									
	RCC->CFGR |= ((pll_freq/OSC_FREQ - 2) << 18);    //设置PLL值 2~16
	RCC->CFGR |= (1 << 16);	  		//设置HSE时钟作为PLL的输入时钟

	FLASH->ACR |= 0x32;	  			//FLASH 2个延时周期 ???

	RCC->CR |= 0x01000000;  		//PLL时钟使能
	while (!(RCC->CR & 0x02000000))
		;							//等待PLL锁定

	RCC->CFGR |= 0x00000002;			//设置PLL为系统时钟	 
	while (!(RCC->CFGR & 0x00000008))   //等待PLL作为系统时钟设置成功
		;
}

///////////////////////////////////////////////////////////////////////////////////
//       							导出函数                                     //
///////////////////////////////////////////////////////////////////////////////////

//设置NVIC 
//NVIC_PreemptionPriority:抢占优先级
//NVIC_SubPriority       :响应优先级
//NVIC_Channel           :中断编号
//NVIC_Group             :中断分组 0~4
//注意优先级不能超过设定的组的范围!否则会有意想不到的错误
//组划分:
//组0:0位抢占优先级,4位响应优先级
//组1:1位抢占优先级,3位响应优先级
//组2:2位抢占优先级,2位响应优先级
//组3:3位抢占优先级,1位响应优先级
//组4:4位抢占优先级,0位响应优先级
//NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,越优先
void nvic_init(u8 NVIC_PreemptionPriority,u8 NVIC_SubPriority,u8 NVIC_Channel,u8 NVIC_Group)	 
{ 
	u32 temp;	
	u8 IPRADDR = NVIC_Channel / 4;  	//每组只能存4个,得到组地址 
	u8 IPROFFSET = NVIC_Channel % 4;	//在组内的偏移

	IPROFFSET = IPROFFSET * 8 + 4;    			//得到偏移的确切位置
	MY_NVIC_PriorityGroupConfig(NVIC_Group);	//设置分组

	temp = (NVIC_PreemptionPriority << (4 - NVIC_Group));	  
	temp |= (NVIC_SubPriority & (0x0f >> NVIC_Group));
	temp &= 0xf;	//取低四位

	if (NVIC_Channel < 32)
		NVIC->ISER[0] |= 1 << NVIC_Channel;		//使能中断位(要清除的话,相反操作就OK)
	else
		NVIC->ISER[1] |= 1 << (NVIC_Channel-32);    
	NVIC->IPR[IPRADDR] |= (temp << IPROFFSET);	//设置响应优先级和抢断优先级   	    	  				   
}

void sys_init(void)
{										  					   
	 MYRCC_DeInit();
	 pll_clock_init(PLL_FREQ);
}

//JTAG模式设置,用于设置JTAG的模式
//mode:jtag,swd模式设置;00,全使能;01,使能SWD;10,全关闭;
//CHECK OK	
//100818		  
void jtag_set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //开启辅助时钟	   
	AFIO->MAPR&=0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR|=temp;       //设置jtag模式
}
