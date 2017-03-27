// Support_DAC_basic_1_2b.c
//
// 05May2015 .. new packaging .. K.Metzger
//			 .. replaced TIM6 (a 16 bit ctr) by TIM2 (a 32 bit ctr)
// 15Oct2015 .. reorganized .. KM
// 07Feb2016 .. rounded timer divide 

//#pragma optimize_level=3

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#ifdef STM32F746xx

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#else
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#endif

// External interface -----------------------------
void DAC_initialize(uint32_t Sample_Rate);
void DAC_start(void);

// Global accessible DAC values
volatile int16_t DAC1_value, DAC2_value, DAC_ready_flag, DAC_overrun_flag;
volatile uint32_t DAC_sample_counter;

//-------------------------------------------------

// Internal defines
void DAC_configure(uint32_t);
void DAC_start_running(void);

void __attribute__((weak)) DAC_ISR_X(void)
{
	return;
}

// Uses timer 2 (32 bit) to set the DAC update rate.

void DAC_initialize(uint32_t Sample_Rate)
{
	DAC_configure(Sample_Rate);
}

void DAC_start(void)
{
	DAC_sample_counter = 0;
	DAC_ready_flag = 0;
	DAC_start_running();
	while(DAC_ready_flag == 0);
	DAC_ready_flag = DAC_overrun_flag = 0;
}

//volatile uint16_t TIM2_SR_value;

void TIM2_IRQHandler(void)
{
	GPIOE->BSRR = GPIO_PIN_9; 	// used to time interrupt duration
	TIM2->SR &= ~0x005F;		// clear flags

 	DAC_ISR_X();				// call extended interrupt support
	if (DAC_ready_flag != 0) DAC_overrun_flag = 1;
	DAC->DHR12L1 = (DAC1_value^0x8000)&0xFFF0;
	DAC->DHR12L2 = (DAC2_value^0x8000)&0xFFF0;
	DAC_ready_flag = 1;
	DAC_sample_counter++;
	//TIM2->SR &= ~0x005F;	// will double trigger on F746
	GPIOE->BSRR = (uint32_t)GPIO_PIN_9<<16;
}


// Timer 2 is used to generate the DAC update clock

void TIM2_configure(uint32_t); // local configuration function

// Counter clocks are derived from 84 MHz.
//
// For 40 kHz transducer
//		Mode 1: sample at 4x40 kHz
// 			84,000,000/160,000 => 525 count down
//			sample clock divider = 10 .. gives 16,000 Hz
// 		Mode 2: sample at 8x40 kHz
//			84,000,000/320,000 => 263 count down -- close enough
//			sample clock divider = 20 .. gives 16,000 Hz
//
// For WWVB 60 kHz -- test output on DAC1 and DAC2
//		Use Mode = 0, sample at 4*60000/5 = 48 kHz
//			84,000,000x5/240000 = 1750

static GPIO_InitTypeDef GPIOA_InitStructure;
static DAC_HandleTypeDef DAC_HandleStructure;
static DAC_ChannelConfTypeDef DAC_ChannelConfStructure;
static TIM_HandleTypeDef TIM2_HandleStructure;

uint32_t DAC1_sample_rate;
uint32_t DAC1_Clock_Divider;

// DAC1_Clk_Divider sets the DAC clock.  It should be
// 84000000 divided by the desired fs (typically 4*fc).

void DAC_configure(uint32_t DAC1_SampleRate)
{
	DAC1_sample_rate = DAC1_SampleRate;
//	DAC1_Clock_Divider = 84000000/DAC1_SampleRate;
	DAC1_Clock_Divider = (SystemCoreClock/2)/DAC1_SampleRate;

	__GPIOA_CLK_ENABLE();	// enable the GPIO port A clock
	__DAC_CLK_ENABLE();		// enable the DAC clock

	// DAC channel 1 & 2 (DAC_OUT1 = PA.4)(DAC_OUT2 = PA.5) pin configuration
	GPIOA_InitStructure.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	GPIOA_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIOA_InitStructure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIOA_InitStructure); // Configure port A for DAC

	DAC_HandleStructure.Instance = DAC;
	HAL_DAC_DeInit(&DAC_HandleStructure);	// set DAC registers to default values
	DAC_ChannelConfStructure.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
	DAC_ChannelConfStructure.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
	HAL_DAC_ConfigChannel(&DAC_HandleStructure, &DAC_ChannelConfStructure, DAC_CHANNEL_1);
	HAL_DAC_ConfigChannel(&DAC_HandleStructure, &DAC_ChannelConfStructure, DAC_CHANNEL_2);
	HAL_DAC_Init(&DAC_HandleStructure);		// now configure the DAC
	HAL_DAC_Start(&DAC_HandleStructure, DAC_CHANNEL_1);
	HAL_DAC_Start(&DAC_HandleStructure, DAC_CHANNEL_2);

	TIM2_configure(DAC1_Clock_Divider); // timer 2 is used to clock the DACs
}

TIM_MasterConfigTypeDef TIM2_MasterConfStructure;

void TIM2_configure(uint32_t Clock_Divider)
{
	__TIM2_CLK_ENABLE();	// enable the timer 2 clock source

	// Time base configuration
	TIM2_HandleStructure.Instance = TIM2;
	HAL_TIM_Base_DeInit(&TIM2_HandleStructure);
	TIM2_HandleStructure.Init.Period = Clock_Divider-1;
	TIM2_HandleStructure.Init.Prescaler = 0;
	TIM2_HandleStructure.Init.ClockDivision = 0;
	TIM2_HandleStructure.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TIM2_HandleStructure);
	TIM2_MasterConfStructure.MasterOutputTrigger = TIM_TRGO_UPDATE;
	TIM2_MasterConfStructure.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&TIM2_HandleStructure, &TIM2_MasterConfStructure);

	// Configure the DAC TIM2 interrupt vector
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3); // three bits priority, one bit sub priority
	HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void DAC_start_running()
{
	HAL_TIM_Base_Start_IT(&TIM2_HandleStructure);
}

void DAC_ARR_update(uint32_t fs)
{
	//TIM2->ARR =  round(84000000/fs)-1;
	TIM2->ARR = round((double)(SystemCoreClock/2)/(double)fs)-1;
	TIM2->CNT = 0;
}
