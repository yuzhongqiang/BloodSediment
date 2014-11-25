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

void _timer_init(u16 freq_div, u16 arr);
void _motor_startup(u8 motor_id);
void _motor_stop(u8 motor_id);
u8 _motor0_is_reset(void);
u8 _motor1_is_reset(void);
u8 _motor2_is_reset(void);
void _motor_set_dir(u8 dir);
u8 _fn_motor_move_steps(void);
u8 _fn_motor0_scan_chn(void);
u8 _fn_motor0_reset_position_blocked(void);
u8 _fn_motor0_reset_position(void);

/*
  MOTOR_ENx:  PA4	- �͵�ƽ��Ч
  MOTOR_DIRx: PA5
  MOTOR_CLKx: PA6 - ��� 100us
*/
#define MOTOR_ENX  PAout(6)	// PA4
#define MOTOR_DIRX PAout(5)	// PA5
#define MOTOR_CLKX PAout(4)	// PA6

/* TIMER��ִ���жϵĺ���*/
TIMER_FN g_timer_fn = NULL;
/* Ҫ���ƶ��Ĳ���*/
static u32 g_demand_steps;   
u8 g_scan_stage = SCAN_STAGE_RESETED;

u32 g_cur_trip0;
u32 g_cur_trip1;
u32 g_cur_trip2;

/* motor1's docking place (from reset position) */
#define POS0      1400
#define POS_INTV 1000
u32 g_docking[10] = {
	POS0,
	POS0 + 1 * POS_INTV,
	POS0 + 2 * POS_INTV,
	POS0 + 3 * POS_INTV,
	POS0 + 4 * POS_INTV,
	POS0 + 5 * POS_INTV,
	POS0 + 6 * POS_INTV,
	POS0 + 7 * POS_INTV,
	POS0 + 8 * POS_INTV,
	POS0 + 9 * POS_INTV
};


//��ʱ��3�жϷ������	 
void TIM3_IRQHandler(void)
{
	u16 temp;

	if (TIM3->SR & 0x0001)  //����ж�
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
	TIM3->SR &= ~(1 << 0);	//����жϱ�־λ
}

void _motor_set_speed(u16 arr)
{
	//TIM3->DIER &= (~(1<<0));    //Disable timer3 interrupt
	//TIM3->CR1 &= ~(1 << 0);;    //�رն�ʱ��3
 	//delay_us(100);
	TIM3->ARR = arr;
	delay_us(100);
}

/*
	��ʱ���ļ���Ƶ�� = 72,000,000/freq_div
	����ֵ = count
*/
void _timer_init(u16 freq_div, u16 arr)
{
	/*
	ͨ�ö�ʱ��3��ʼ����ʱ��ѡ��ΪAPB2(72MHz)
		arr��		�Զ���װֵ
		freq_div��	ʱ��Ԥ��Ƶ��	
	*/
	RCC->APB1ENR |= (1 << 1);	// TIMER3ʱ��ʹ��    
 	TIM3->ARR = arr;  			// �趨�������Զ���װֵ//�պ�1ms    
	TIM3->PSC = freq_div - 1;	// Ԥ��Ƶ��7200,�õ�10Khz�ļ���ʱ��
	//TIM3->DIER |= (1 << 0);   	//��������ж�		
	
  	nvic_init(1,3, TIM3_IRQChannel,2);//��ռ1�������ȼ�3����2
}

void _motor_startup(u8 motor_id)
{	
	GPIOC->ODR |= (1 << motor_id);  /* MT_x */
	//GPIOC->ODR |= 0x07;  /* ONLY for test */
	delay_us(100);
	GPIOA->ODR &= 0xffBf;     //ENx�õͣ��򿪵��
	delay_us(100);

	TIM3->DIER |= (1 << 0);   	//��������ж�		
	TIM3->CR1 |= 0x01;      	//ʹ�ܶ�ʱ��3
	delay_us(100);
}

void _motor_stop(u8 motor_id)
{
	TIM3->DIER &= (~(1<<0));    //Disable timer3 interrupt
	TIM3->CR1 &= ~(1 << 0);;    //�رն�ʱ��3
 	delay_us(100);
	
 	GPIOA->ODR |= 0x0040;	//ENx�øߣ��رյ��
 	delay_us(100);
	GPIOC->ODR &= (~(1 << motor_id));
	delay_us(100);
}

/* ����ֵ��
	1 - ������е����·�
	0 - ���û�����е����·�
*/
u8 _motor0_is_reset(void)
{
	/* A_INIT: PB5 */
	u16 pcv = GPIOB->IDR;
  	return (pcv & 0x0020) ? 1 : 0;
}

/* ����ֵ��
	1 - ������е����·�
	0 - ���û�����е����·�
*/
u8 _motor1_is_reset(void)
{
	/* B_INIT: PB8 */
	u16 pcv = GPIOB->IDR;
  	return (pcv & 0x0100) ? 1 : 0;
}

/* ����ֵ��
	1 - ������е����·�
	0 - ���û�����е����·�
*/
static u8 _motor2_is_reset(void)
{
	/* C_INIT: PB9 */
	u16 pcv = GPIOB->IDR;
  	return (pcv & 0x0200) ? 1 : 0;
}

 /* ���õ������
    0 - ����
	1 - ����
*/
void _motor_set_dir(u8 dir)
{
	if (!dir)
		GPIOA->ODR |= 0x0020;
	else
		GPIOA->ODR &= 0xffdf;	   
}

/**************************************************************
					Motor move specific step routines
**************************************************************/

u8 _fn_motor_move_steps(void)
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
	_motor_stop(motor_id);
}

void motor2_shake(u32 steps)
{
	u8 i;

	for (i=0; i<8; i++)
	{
		g_timer_fn = _fn_motor_move_steps;
		g_demand_steps = steps;
		_motor_set_dir(1);
		_motor_startup(2);	

		/* blocking ... */
		while (g_demand_steps > 0)
			;
		delay_ms(500);
		motor_reset_position_blocked(2);
	}
}

/*
	0xff: Reset position
	0 ~ 9: Working position
*/
u8 g_motor1_pos = 0xff; 
extern struct tube tubes[MAX_CHANNELS];

void _motor_check_shake(void)
{
	u8 i, dir;
	u32 steps;
	
	for (i=0; i<MAX_CHANNELS; i++) {
		/* do shaking when scan the first time */
		if ((tubes[i].inplace == 1) &&
			(tubes[i].remains == MAX_MEASURE_TIMES))
		{
			/* ��������ٶ�*/
			_motor_set_speed(5 * 6);
			
			/* motor1 reach to resetting place */
			if (g_motor1_pos == 0xff)
			{
				steps = g_docking[i];
				dir = MOTOR0_DIR_FWD;
			}
			else if (i > g_motor1_pos)
			{
				steps = g_docking[i] - g_docking[g_motor1_pos];
				dir = MOTOR0_DIR_FWD;
			}
			else
			{
				steps = g_docking[g_motor1_pos] - g_docking[i];
				dir = MOTOR0_DIR_BWD;
			}
			motor_move_steps_blocked(1, dir, steps);
			delay_ms(10);		
			motor2_shake(18);
		
			/* ����ٶȻ�ԭ*/
			_motor_set_speed(5);
			g_motor1_pos = i;
		}
		
	}
}

/**************************************************************
						Motor reset routines(blocked)
**************************************************************/
static u8 _fn_motor0_reset_position_blocked(void)
{
	if (_motor0_is_reset())
	{
		g_demand_steps = 0;
		g_cur_trip0 = 0;
		g_scan_stage = SCAN_STAGE_RESETED;
		return 0;
	}
	else
	{
		g_demand_steps = 1;
		return 1;
	}
}

static u8 _fn_motor1_reset_position_blocked(void)
{
	if (_motor1_is_reset())
	{
		g_demand_steps = 0;
		return 0;
	}
	else
	{
		g_demand_steps = 1;
		return 1;
	}
}

static u8 _fn_motor2_reset_position_blocked(void)
{
	if (_motor2_is_reset())
	{
		g_demand_steps = 0;
		return 0;
	}
	else
	{
		g_demand_steps = 1;
		return 1;
	}
}

/**************************************************************
			  Motor reset routines (non blocked)
**************************************************************/
#if 0
static u8 _fn_motor0_reset_position(void)
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

static u8 _fn_motor1_reset_position(void)
{
	if (_motor1_is_reset())
		return 0;
	else
		return 1;
}

static u8 _fn_motor2_reset_position(void)
{
	if (_motor2_is_reset())
		return 0;
	else
		return 1;
}
#endif

void motor_reset_position_blocked(u8 motor_id)
{
	switch (motor_id)
	{
	case 0:
		if (_motor0_is_reset())
			motor_move_steps_blocked(motor_id, MOTOR0_DIR_UP, 600);

		g_demand_steps = 1;
		g_timer_fn = _fn_motor0_reset_position_blocked;
		_motor_set_dir(MOTOR0_DIR_DOWN);
		_motor_startup(motor_id);	
		break;
	case 1:
		if (_motor1_is_reset())
			motor_move_steps_blocked(motor_id, MOTOR0_DIR_FWD, 600);

		g_demand_steps = 1;
		g_timer_fn = _fn_motor1_reset_position_blocked;
		_motor_set_dir(MOTOR0_DIR_BWD);
		_motor_startup(motor_id);	
		break;
	case 2:
		if (_motor2_is_reset())
			motor_move_steps_blocked(motor_id, MOTOR0_DIR_PUSH, 10);

		g_demand_steps = 1;
		g_timer_fn = _fn_motor2_reset_position_blocked;
		_motor_set_dir(MOTOR0_DIR_RELEASE);
		_motor_startup(motor_id);	
		break;
	default:
		break;
	}

	/* blocking ... */
	while (g_demand_steps > 0)
		;
	_motor_stop(motor_id);
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

/*
  Ret value: 0 - scanning complete
  Ret value: 1 - scanning not complete
*/
static u8 _fn_motor0_scan_chn(void)
{
	u16 temp;
	static u8 cur_chn = 0;
	u8 i;

	if (tubes[cur_chn].inplace == 0) {
		cur_chn = ((cur_chn + 1) % MAX_CHANNELS);
		return 1;
	}
	
	temp = GPIOA->ODR;
	if (!(temp & 0x0010)) { //CLKX is low
		g_cur_trip0++;
		if ((channel_is_opaque(cur_chn)))
			tubes[cur_chn].values[13 - tubes[cur_chn].remains] = g_cur_trip0;	
		
		if (g_cur_trip0 == MOTOR0_MAX_TRIP)
			goto should_stop;

	}
	cur_chn = ((cur_chn +1 ) % MAX_CHANNELS);
	return 1;
	
should_stop:
	for (i=0; i<MAX_CHANNELS; i++) {
		if (tubes[i].inplace == 0) {
			tubes[i].remains--;
			tubes[i].last_scan_time = rtc_get_sec();
		}
	}
	g_scan_stage = SCAN_STAGE_SCANFINISH;
	g_cur_trip0 = 0;
	g_demand_steps = 0;
		
	return 0;		
}

/* In fact, chn_id is not used here now */
void motor_scan_chn(u8 motor_id)
{
	/* start schan ... */	
	g_scan_stage = SCAN_STAGE_SCANNING;
	g_timer_fn = _fn_motor0_scan_chn;
	g_demand_steps = 1;

	_motor_set_dir(MOTOR0_DIR_UP);
	_motor_startup(0);
	while (g_demand_steps > 0)
		;
	_motor_stop(0);
}

/*
  GPIOA��sys.c���Ѿ�ʹ��
  ����PA4, PA5, PA6Ϊͨ�����������ʽ
*/
void motor_init(void)
{
	_timer_init(7200, 5);
	delay_ms(10);

	/* 
	    MT_1(PC0), MT_2(PC1), MT_3(PC2) initialization,
	*/
	GPIOC->CRL &= 0xFFFFF000;
	GPIOC->CRL |= 0x00000333;
	/* First enable it */
	GPIOC->ODR |= 0x0007;	
	
	/*
	ENx(PA6), DIRx(PA5), CLKx(PA4)�����ģʽ 
	��ʼʱ����ߵ�ƽ��Enx low active 
	*/
	GPIOA->CRL &= 0xF000FFFF;
	GPIOA->CRL |= 0x03330000;
	GPIOA->ODR |= 0x0040;

	/* Disable MT_1, MT_2, MT_3 */
	GPIOC->ODR &= 0xfff8;

	/* ����A_INIT(PB5), B_INIT(PB8), C_INIT(PB9)Ϊ����/��������ģʽ */
	GPIOB->CRL &= 0xFF0FFFFF;
	GPIOB->CRL |= 0x00800000;
	GPIOB->CRH &= 0xFFFFFF00;
	GPIOB->CRH |= 0x00000088;
									
	// ����ظ�����ʼλ��
	motor_reset_position_blocked(2);
	motor_reset_position_blocked(0);
	motor_reset_position_blocked(1);
}

