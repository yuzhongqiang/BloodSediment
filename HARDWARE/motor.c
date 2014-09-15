/*
* motor.c
*/

#include <stm32f10x_lib.h>
#include <stdio.h>
#include "sys.h"
#include "motor.h"
#include "channel.h"
#include "delay.h"
#include "rtc.h"

static void _timer_init(u16 freq_div, u16 arr);
static void _motor_startup(u8 motor_id);
static void _motor_stop(u8 motor_id);
static u8 _motor0_is_reset(void);
static u8 _motor1_is_reset(void);
static u8 _motor2_is_reset(void);
static void _motor_set_dir(u8 dir);
static u8 _fn_motor_move_steps(void);
static u8 _fn_motor0_scan_chn(void);
static u8 _fn_motor0_reset_position_blocked(void);
#if 0
static u8 _fn_motor0_reset_position(void);
#endif

/*
  MOTOR_ENx:  PA4	- 低电平有效
  MOTOR_DIRx: PA5
  MOTOR_CLKx: PA6 - 最快 100us
*/
#define MOTOR_ENX  PAout(6)	// PA4
#define MOTOR_DIRX PAout(5)	// PA5
#define MOTOR_CLKX PAout(4)	// PA6

/* TIMER中执行中断的函数*/
TIMER_FN g_timer_fn = NULL;
/* 要求移动的步数*/
static u32 g_demand_steps;   
u8 g_scan_stage = SCAN_STAGE_INITED;

u32 g_cur_trip0;
u32 g_cur_trip1;
u32 g_cur_trip2;


//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{
	u16 temp;

	if (TIM3->SR & 0x0001)  //溢出中断
	{
		if (g_timer_fn != NULL)
		if (g_timer_fn() == 0)
		{
			TIM3->DIER &= (~(1 << 0));   	//Disable timer3 interrupt
			g_timer_fn = NULL;
			goto clear;
		}

		temp = GPIOA->ODR;
		temp ^= 0x0010;
		GPIOA->ODR = temp;	
	}
	
clear:	
	TIM3->SR &= ~(1 << 0);	//清除中断标志位
}

/*
	定时器的计数频率 = 72,000,000/freq_div
	计数值 = count
*/
static void _timer_init(u16 freq_div, u16 arr)
{
	/*
	通用定时器3初始化，时钟选择为APB2(72MHz)
		arr：		自动重装值
		freq_div：	时钟预分频数	
	*/
	RCC->APB1ENR |= (1 << 1);	// TIMER3时钟使能    
 	TIM3->ARR = arr;  			// 设定计数器自动重装值//刚好1ms    
	TIM3->PSC = freq_div - 1;	// 预分频器7200,得到10Khz的计数时钟
	//TIM3->DIER |= (1 << 0);   	//允许更新中断		
	
  	nvic_init(1,3, TIM3_IRQChannel,2);//抢占1，子优先级3，组2
}

static void _motor_startup(u8 motor_id)
{	
	GPIOC->ODR |= (1 << motor_id);
	delay_ms(1);

	TIM3->DIER |= (1 << 0);   	//允许更新中断		
	GPIOA->ODR &= 0xffBf;     //ENx置低，关闭电机
	TIM3->CR1 |= 0x01;      	//使能定时器3
}

static void _motor_stop(u8 motor_id)
{
	GPIOC->ODR &= (~(1 << motor_id));
	delay_ms(1);

	TIM3->DIER &= (~(1<<0));    //Disable timer3 interrupt
	TIM3->CR1 &= ~(1 << 0);;    //关闭定时器3
 	GPIOA->ODR |= 0x0040;	//ENx置高，关闭电机
}


/* 返回值：
	1 - 电机运行到最下方
	0 - 电机没有运行到最下方
*/
static u8 _motor0_is_reset(void)
{
	u16 pcv = GPIOC->IDR;
  	return (pcv & 0x0080) ? 1 : 0;
}

/* 返回值：
	1 - 电机运行到最下方
	0 - 电机没有运行到最下方
*/
static u8 _motor1_is_reset(void)
{
	u16 pcv = GPIOC->IDR;
  	return (pcv & 0x0080) ? 1 : 0;
}

/* 返回值：
	1 - 电机运行到最下方
	0 - 电机没有运行到最下方
*/
static u8 _motor2_is_reset(void)
{
	u16 pcv = GPIOC->IDR;
  	return (pcv & 0x0080) ? 1 : 0;
}

 /* 设置电机方向
    0 - 向下
	1 - 向上
*/
static void _motor_set_dir(u8 dir)
{
	if (!dir)
		GPIOA->ODR |= 0x0020;
	else
		GPIOA->ODR &= 0xffdf;	   
}

/**************************************************************
					Motor move specific step routines
**************************************************************/

static u8 _fn_motor_move_steps(void)
{
	u16 temp;

	temp = GPIOA->ODR;
	if (!(temp & 0x0010))
	{
		if (g_demand_steps-- ==0)
			return 0;
	}
	return 1;
}

void motor_move_steps(u8 motor_id, u8 dir, u32 steps)
{
	g_timer_fn = _fn_motor_move_steps;
	g_demand_steps = steps;
	_motor_set_dir(dir);
	_motor_startup(motor_id);	
}

void motor_move_steps_blocked(u8 motor_id, u8 dir, u32 steps)
{
	g_timer_fn = _fn_motor_move_steps;
	g_demand_steps = steps;
	_motor_set_dir(dir);
	_motor_startup(motor_id);	

	/* blocking ... */
	while (g_demand_steps > 0)
		;
}

/**************************************************************
						Motor reset routines(blocked)
**************************************************************/
static u8 _fn_motor0_reset_position_blocked(void)
{
	if (_motor0_is_reset())
	{
		g_cur_trip0 = 0;
		g_scan_stage = SCAN_STAGE_RESETED;
		return 0;
	}
	else
		return 1;
}

void motor_reset_position_blocked(u8 motor_id)
{
	switch (motor_id)
	{
	case 0:
		if (_motor0_is_reset())
			motor_move_steps_blocked(0, MOTOR0_DIR_UP, 200);
		
		g_timer_fn = _fn_motor0_reset_position_blocked;
		_motor_set_dir(MOTOR0_DIR_DOWN);
		_motor_startup(motor_id);			
		break;
	case 1:
		break;
	case 2:
		break;
	default:
		break;
	}
}

#if 0
/**************************************************************
					Motor reset routines(non block)
**************************************************************/
static u8 _fn_motor0_reset_position(void)
{
	if (_motor0_is_reset())
	{
		g_cur_trip0 = 0;
		g_scan_stage = SCAN_STAGE_SCANFINISH;
		return 0;
	}
	else
		return 1;
}

void motor_reset_position(u8 motor_id)
{
	switch (motor_id)
	{
	case 0:
		g_timer_fn = _fn_motor0_reset_position;
		_motor_set_dir(MOTOR0_DIR_DOWN);
		_motor_startup(motor_id);			
		break;
	case 1:
		break;
	case 2:
		break;
	default:
		break;
	}
}
#endif

/**************************************************************
						Motor reset routines
**************************************************************/

extern struct tube tubes[MAX_CHANNELS];
extern u8 g_cur_chn;
extern u8 channel_is_opaque(u8 chn);

static u8 _fn_motor0_scan_chn(void)
{
	u16 temp;

	temp = GPIOA->ODR;
	if (!(temp & 0x0010))
	{
		if (channel_is_opaque(g_cur_chn))
		{
			g_cur_trip0++;
			if (g_cur_trip0 == MOTOR0_MAX_TRIP)
				goto should_stop;
		}
		else
			goto should_stop;
	}
	return 1;

should_stop:
	tubes[g_cur_chn].values[13 - tubes[g_cur_chn].remains] = g_cur_trip0;
	tubes[g_cur_chn].remains--;
	tubes[g_cur_chn].last_scan_time = rtc_get_sec();
	g_scan_stage = SCAN_STAGE_SCANFINISH;
	return 0;		
}

void motor_scan_chn(u8 motor_id, u8 chn_id)
{
	g_scan_stage = SCAN_STAGE_SCANNING;
	g_timer_fn = _fn_motor0_scan_chn;
	tubes[chn_id].scan_times[13 - tubes[chn_id].remains] = rtc_get_sec();
	
	_motor_set_dir(MOTOR0_DIR_UP);
	_motor_startup(motor_id);	
}

/*
  GPIOA在sys.c中已经使能
  配置PA4, PA5, PA6为通用推挽输出方式
*/
void motor_init(void)
{
	_timer_init(7200, 1);
	delay_ms(10);
	
	/* ENx(PA6), DIRx(PA5), CLKx(PA4)，输出模式 */
	GPIOA->CRL &= 0xF000FFFF;
	GPIOA->CRL |= 0x03330000;

	/* 初始时输出高电平，Enx low active */
	GPIOA->ODR |= 0x0040;

	/* MT_1(PC0), MT_2(PC1), MT_3(PC2) initialization, disable */
	GPIOC->CRL &= 0xFFFFF000;
	GPIOC->CRL |= 0x00000333;
	GPIOC->ODR &= 0xfff8;	
									
	// 电机回复到起始位置
	motor_reset_position_blocked(0);
	//motor_reset_position(1);
	//motor_reset_position(2);
}

