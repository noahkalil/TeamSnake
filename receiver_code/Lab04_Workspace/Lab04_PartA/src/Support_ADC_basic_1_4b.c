// File name: ADC_basic_support_1_4b.c
//
// 27Apr2015 .. initial version .. K.Metzger
// 29Mar2016 .. added ADC 3 .. KM
// 12Jun2016 .. added ST32F746 support .. KM

#pragma optimize_level=3

#include <stdint.h>

#ifdef STM32F746xx

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#define ADC1_CHANNEL ADC_CHANNEL_9
#define ADC2_CHANNEL ADC_CHANNEL_12
#define ADC3_CHANNEL ADC_CHANNEL_14
#else
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#define ADC1_CHANNEL ADC_CHANNEL_11
#define ADC2_CHANNEL ADC_CHANNEL_12
#define ADC3_CHANNEL ADC_CHANNEL_13

#endif

void ADC_basic_configure(uint32_t);
void ADC_start_running(void);

// give global accessible to ADC values
volatile int16_t ADC1_value, ADC2_value, ADC3_value, ADC_ready_flag, ADC_overrun_flag;
volatile uint32_t ADC_sample_counter;

static uint32_t ADC_Sample_Rate;


void __attribute__((weak)) ADC_ISR_X(void)  // extend ADC ISR functionality
{
	return;
}

// If sample rate is 0 then uses timer 2 (32-bit) as set up by DAC support.
// Timer 3 (16-bit) is used when the specified sample rate is non-zero.

void ADC_initialize(uint32_t Sample_Rate)	// configure ADC using this sample rate
{
	ADC_basic_configure(Sample_Rate);
}

void ADC_start(void)
{
	ADC_sample_counter = 0;					// init sample counter
	ADC_ready_flag = 0;						// init the ready flag
	ADC_start_running();					// enable interrupts to function
	while(ADC_ready_flag == 0);				// syncing to first sample
	ADC_ready_flag = ADC_overrun_flag = 0;
}

// interrupts are triggered by ADC1 only

void ADC_IRQHandler(void)
{
	GPIOE->BSRR = GPIO_PIN_10; 				// used to time interrupt
	ADC1->SR &= ~((uint32_t)ADC_FLAG_JEOC);	// clear interrupt flag

	ADC1_value = ((int16_t)(ADC1->JDR1)<<4)+0x8000;
	ADC2_value = ((int16_t)(ADC2->JDR1)<<4)+0x8000;
	ADC3_value = ((int16_t)(ADC3->JDR1)<<4)+0x8000;

	if (ADC_ready_flag != 0) ADC_overrun_flag = 1;
	ADC_ready_flag = 1;
	ADC_sample_counter++;

	ADC_ISR_X();	// call extended user interrupt support

	GPIOE->BSRR = (uint32_t)GPIO_PIN_10 << 16;
}

// ADC setup and initialization

void TIM5_Config(uint32_t); // used to set sample rate separate from DAC rate

static GPIO_InitTypeDef GPIOB_InitStructure;
static GPIO_InitTypeDef GPIOC_InitStructure;
static GPIO_InitTypeDef GPIOF_InitStructure;
static ADC_InjectionConfTypeDef ADC_IChannelConfigStructure;
static ADC_HandleTypeDef ADC1_HandleStructure;
static ADC_HandleTypeDef ADC2_HandleStructure;
static ADC_HandleTypeDef ADC3_HandleStructure;
static TIM_HandleTypeDef TIM5_HandleStructure;


void ADC_basic_configure(uint32_t adc_sample_rate)  // internal
{
	uint32_t Trigger;

	ADC_Sample_Rate = adc_sample_rate;

	__ADC1_CLK_ENABLE();
	__ADC2_CLK_ENABLE();
	__ADC3_CLK_ENABLE();

#ifdef STM32F746xx
	__GPIOB_CLK_ENABLE();	// ADC1 IN9
	GPIOB_InitStructure.Pin = GPIO_PIN_1;
	GPIOB_InitStructure.Mode = GPIO_MODE_ANALOG; 	// PB1 is now analog
	GPIOB_InitStructure.Pull = GPIO_NOPULL; 		// don't need any pull up or pull down
	HAL_GPIO_Init(GPIOB,&GPIOB_InitStructure); 		// configure port B

	__GPIOC_CLK_ENABLE();	// ADC2 IN12
	GPIOC_InitStructure.Pin = GPIO_PIN_2;
	GPIOC_InitStructure.Mode = GPIO_MODE_ANALOG; 	// PC2 is now analog
	GPIOC_InitStructure.Pull = GPIO_NOPULL; 		// don't need any pull up or pull down
	HAL_GPIO_Init(GPIOC,&GPIOC_InitStructure); 		// configure port C

	__GPIOF_CLK_ENABLE();	// ADC3 IN14
	GPIOF_InitStructure.Pin = GPIO_PIN_4;
	GPIOF_InitStructure.Mode = GPIO_MODE_ANALOG; 	// PF4 is now analog
	GPIOF_InitStructure.Pull = GPIO_NOPULL; 		// don't need any pull up or pull down
	HAL_GPIO_Init(GPIOF,&GPIOF_InitStructure); 		// configure port F

#else
	__GPIOC_CLK_ENABLE();	//  all ADC channels are on port C

	// ADC 1, 2 and 3 configuration -- Port C
	GPIOC_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIOC_InitStructure.Mode = GPIO_MODE_ANALOG; 	// PC1, PC2 and PC3 pins are now analog
	GPIOC_InitStructure.Pull = GPIO_NOPULL; 		// don't need any pull up or pull down
	HAL_GPIO_Init(GPIOC,&GPIOC_InitStructure); 		// configure port C
#endif

	// Configure the ADC interrupt vector

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3); // three bits priority, one bit sub priority
	HAL_NVIC_SetPriority(ADC_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);

	// Configure ADCs 1, 2 and 3

	if (ADC_Sample_Rate != 0) 	Trigger = ADC_EXTERNALTRIGINJECCONV_T5_TRGO;	// using timer 5
	else 						Trigger = ADC_EXTERNALTRIGINJECCONV_T2_TRGO;	// using DAC's timer

	ADC1_HandleStructure.Instance = ADC1;
	HAL_ADC_DeInit(&ADC1_HandleStructure);
	ADC1_HandleStructure.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	ADC1_HandleStructure.Init.Resolution = ADC_RESOLUTION_12B;
	ADC1_HandleStructure.Init.NbrOfConversion = 1;
	HAL_ADC_Init(&ADC1_HandleStructure);

	ADC_IChannelConfigStructure.InjectedChannel = ADC1_CHANNEL;
	ADC_IChannelConfigStructure.InjectedRank = 1;
	ADC_IChannelConfigStructure.InjectedSamplingTime = ADC_SAMPLETIME_28CYCLES;
	ADC_IChannelConfigStructure.ExternalTrigInjecConv = Trigger;
	ADC_IChannelConfigStructure.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONVEDGE_RISING;
	ADC_IChannelConfigStructure.InjectedNbrOfConversion = 1;
	HAL_ADCEx_InjectedConfigChannel(&ADC1_HandleStructure, &ADC_IChannelConfigStructure);

	ADC2_HandleStructure.Instance = ADC2;
	HAL_ADC_DeInit(&ADC2_HandleStructure);
	ADC2_HandleStructure.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	ADC2_HandleStructure.Init.Resolution = ADC_RESOLUTION_12B;
	ADC2_HandleStructure.Init.NbrOfConversion = 1;
	HAL_ADC_Init(&ADC2_HandleStructure);

	ADC_IChannelConfigStructure.InjectedChannel = ADC2_CHANNEL;
	ADC_IChannelConfigStructure.InjectedRank = 1;
	ADC_IChannelConfigStructure.InjectedSamplingTime = ADC_SAMPLETIME_28CYCLES;
	ADC_IChannelConfigStructure.ExternalTrigInjecConv = Trigger;
	ADC_IChannelConfigStructure.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONVEDGE_RISING;
	ADC_IChannelConfigStructure.InjectedNbrOfConversion = 1;
	HAL_ADCEx_InjectedConfigChannel(&ADC2_HandleStructure, &ADC_IChannelConfigStructure);

	ADC3_HandleStructure.Instance = ADC3;
	HAL_ADC_DeInit(&ADC3_HandleStructure);
	ADC3_HandleStructure.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	ADC3_HandleStructure.Init.Resolution = ADC_RESOLUTION_12B;
	ADC3_HandleStructure.Init.NbrOfConversion = 1;
	HAL_ADC_Init(&ADC3_HandleStructure);

	ADC_IChannelConfigStructure.InjectedChannel = ADC3_CHANNEL;
	ADC_IChannelConfigStructure.InjectedRank = 1;
	ADC_IChannelConfigStructure.InjectedSamplingTime = ADC_SAMPLETIME_28CYCLES;
	ADC_IChannelConfigStructure.ExternalTrigInjecConv = Trigger;
	ADC_IChannelConfigStructure.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONVEDGE_RISING;
	ADC_IChannelConfigStructure.InjectedNbrOfConversion = 1;
	HAL_ADCEx_InjectedConfigChannel(&ADC3_HandleStructure, &ADC_IChannelConfigStructure);

	if (ADC_Sample_Rate != 0) {
		TIM5_Config((SystemCoreClock/2)/ADC_Sample_Rate);  // configure timer 5 to generate sample clock
	}
}

void ADC_start_running(void)
{
	HAL_ADCEx_InjectedStart(&ADC3_HandleStructure);
	HAL_ADCEx_InjectedStart(&ADC2_HandleStructure);
	HAL_ADCEx_InjectedStart_IT(&ADC1_HandleStructure);	// only ADC1 generates interrupts
}

// Timer 5 can be used to trigger AD conversions if not using DAC clock.

TIM_MasterConfigTypeDef TIM5_MasterConfStructure;

void TIM5_Config(uint32_t ADC_Divide_Factor)
{
	__TIM5_CLK_ENABLE();	// enable the timer 5 clock

	/* Time base configuration */
	TIM5_HandleStructure.Instance = TIM5;
	HAL_TIM_Base_DeInit(&TIM5_HandleStructure);
	TIM5_HandleStructure.Init.Period = ADC_Divide_Factor-1;
	TIM5_HandleStructure.Init.Prescaler = 0;
	TIM5_HandleStructure.Init.ClockDivision = 0;
	TIM5_HandleStructure.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&TIM5_HandleStructure);

	TIM5_MasterConfStructure.MasterOutputTrigger = TIM_TRGO_UPDATE;
	TIM5_MasterConfStructure.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&TIM5_HandleStructure, &TIM5_MasterConfStructure);

	// Configure the DAC TIM2 interrupt vector .. only used for testing
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3); // three bits priority, one bit sub priority
	HAL_NVIC_SetPriority(TIM5_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(TIM5_IRQn);

	HAL_TIM_Base_Start(&TIM5_HandleStructure);  // add _IT for testing using interrupt
}

void TIM5_IRQHandler(void)	// only used to verify that counter is running
{
	uint16_t ctr;
	GPIOE->BSRR = GPIO_PIN_8; 	// used to time interrupt duration
	TIM5->SR &= ~0x005F;
	for (ctr = 0; ctr<2; ctr++);
	GPIOE->BSRR = (uint32_t)GPIO_PIN_8<<16;
}

