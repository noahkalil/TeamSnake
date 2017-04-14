// File name: Pin_suport_1_3.c
//

#ifdef STM32F746xx
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#else
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#endif

#include <stdint.h>


static GPIO_InitTypeDef GPIOA_InitStructure;
static GPIO_InitTypeDef GPIOC_InitStructure;
static GPIO_InitTypeDef GPIOD_InitStructure;
static GPIO_InitTypeDef GPIOE_InitStructure;

// Initializes:  DAC and DAC interval pins
//				 User whatever pins
//               push button
//               user LEDs

void Pins_initialize()
{
	// Initialize GIPO port A pin 0 .. user switch
	//
	__GPIOA_CLK_ENABLE();
	GPIOA_InitStructure.Pin = GPIO_PIN_0;
	GPIOA_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIOA_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	GPIOA_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA, &GPIOA_InitStructure);

	// Initialize GIPO port C pin ENGIN100-850 4-bit DAC
	//
	__GPIOC_CLK_ENABLE();
	GPIOC_InitStructure.Pin = GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13;
	GPIOC_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIOC_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIOC_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC, &GPIOC_InitStructure);
	
	//  Initialize GPIO port D LED pins
	//
	__GPIOD_CLK_ENABLE();
	GPIOD_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	// blue--15, red--14, orange--13, green--12
	GPIOD_InitStructure.Pin =  GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_13|GPIO_PIN_12;
	GPIOD_InitStructure.Pull = GPIO_NOPULL;
	GPIOD_InitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOD,&GPIOD_InitStructure);
	
	//  Initialize GPIO port E test pins
	//
	__GPIOE_CLK_ENABLE();
	GPIOE_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIOE_InitStructure.Pin =  GPIO_PIN_12|GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8|
				GPIO_PIN_6; // pin 6 is for 4-bit DAC
	GPIOE_InitStructure.Pull = GPIO_PULLUP;
	GPIOE_InitStructure.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOE,&GPIOE_InitStructure);
}

void LED_Green_On()
{
	GPIOD->ODR |= GPIO_PIN_12;
}

void LED_Green_Off()
{
	GPIOD->ODR &= ~GPIO_PIN_12;
}

void LED_Green_Toggle()
{
	GPIOD->ODR ^= GPIO_PIN_12;
}

void LED_Orange_On()
{
	GPIOD->ODR |= GPIO_PIN_13;
}

void LED_Orange_Off()
{
	GPIOD->ODR &= ~GPIO_PIN_13;
}

void LED_Orange_Toggle()
{
	GPIOD->ODR ^= GPIO_PIN_13;
}

void LED_Red_On()
{
	GPIOD->ODR |= GPIO_PIN_14;
}

void LED_Red_Off()
{
	GPIOD->ODR &= ~GPIO_PIN_14;
}

void LED_Red_Toggle()
{
	GPIOD->ODR ^= GPIO_PIN_14;
}

void LED_Blue_On()
{
	GPIOD->ODR |= GPIO_PIN_15;
}

void LED_Blue_Off()
{
	GPIOD->ODR &= ~GPIO_PIN_15;
}

void LED_Blue_Toggle()
{
	GPIOD->ODR ^= GPIO_PIN_15;
}

// test pins

int Pin_E11(void)
{
	//if (value != 0) GPIOE->ODR |= GPIO_PIN_11;
	//else GPIOE->ODR &= ~GPIO_PIN_11;
	return (~GPIOE->IDR & GPIO_PIN_11);
}

void Pin_E12(uint16_t value)
{
	if (value != 0) GPIOE->ODR |= GPIO_PIN_12;
	else GPIOE->ODR &= ~GPIO_PIN_12;
}

uint16_t UserButton()
{
	return GPIOA->IDR & 0x1;
}

// ENGIN100 Winter 2016 DAC bits

int Pin_PC14(void)
{
	// PC14

	//if (value != 0) GPIOC->ODR |= GPIO_PIN_14;
	//else GPIOC->ODR &= ~GPIO_PIN_14;
	return (~GPIOC->IDR & GPIO_PIN_14) ? 0 : 1;
}

int Pin_PC15(void)
{
	// PC15

	//if (value != 0) GPIOC->ODR |= GPIO_PIN_15;
	//else GPIOC->ODR &= ~GPIO_PIN_15;
	
	return (~GPIOC->IDR & GPIO_PIN_15) ? 0 : 1;
}

int Pin_PE6(void)
{
	// PE6

	//if (value != 0) GPIOE->ODR |= GPIO_PIN_6;
	//else GPIOE->ODR &= ~GPIO_PIN_6;
	return (~GPIOE->IDR & GPIO_PIN_6) ? 0 : 1;
}

int Pin_PC13(void)
{
	// PC13

	//if (value != 0) GPIOC->ODR |= GPIO_PIN_13;
	//else GPIOC->ODR &= ~GPIO_PIN_13;
	return (~GPIOC->IDR & GPIO_PIN_13) ? 0 : 1;
}

int8_t Button_pressed(void) {
    // up
    if ( Pin_PC14() )
      return 0x0;

    // down
    if ( Pin_PC15() )
      return 0x1;

    // left
    if ( Pin_PE6() ) 
      return 0x2;

    // right
    if ( Pin_PC13() ) 
      return 0x3;
      
    // no button / no transmission
    return -1;
}
