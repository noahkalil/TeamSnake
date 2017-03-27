// File name: USART_support_1_7b.c
//
// Only the USART Rx input runs under interrupts
//
// 07May2014 .. initial version started .. K.Metzger
// 21May2014 .. configured for scanf input and receive interrupts .. KM
// 19Aug2014 .. added GPIO pin E8 for timing interrupts .. KM
// 26Oct2014 .. declared EscapeFlag as volatile .. KM
// 29Oct2014 .. renamed using V1.1 .. KM
// 25Oct2014 .. redid not using HAL, moved RTS output to Port B .. KM
// 06May2015 .. simplified interrupt support and added peek ability .. KM
// 30Sep2015 .. added sw_rtscts to initialize call .. KM
// 29Mar2016 .. when sw_rtscts == 0 hold rts low .. KM
// 12Apr2016 .. receiver flow RTS/CTS added .. most every else is broken .. KM

#include <stdio.h>
#include <stdint.h>

#ifdef STM32F746xx

#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"
#define USART_SH USART2
#define USART_SH_IRQHandler USART2_IRQHandler
#define SET_RTS_HIGH GPIOD->ODR &= ~GPIO_PIN_4
#define SET_RTS_LOW  GPIOD->ODR |= GPIO_PIN_4

#else	// STM32F4

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#define USART_SH USART3
#define USART_SH_IRQHandler USART3_IRQHandler
#define SET_RTS_HIGH GPIOB->ODR &= ~GPIO_PIN_14
#define SET_RTS_LOW  GPIOB->ODR |= GPIO_PIN_14
#define ISR SR
#define TDR DR
#define RDR DR

#endif

#include "FIFO_builder.h"

// Create FIFO for USART receive data.  SIZE must be power of 2.
// Creates: USART_Fifo_Init, USART_Fifo_Get, USART_Fifo_Put, USART_Fifo_Size.

#define USART_FIFO_SIZE 256
AddIndexFifo(USART_, USART_FIFO_SIZE, uint8_t, 1, 0);

volatile int16_t EscapeFlag;
volatile int16_t USART_flag;
volatile uint16_t USART_char;

void USART_initialize(uint32_t baudrate, uint8_t sw_rtscts)
{
	uint32_t temp;

	EscapeFlag = 0;

#ifdef STM32F746xx
	RCC->DCKCFGR2 |= RCC_DCKCFGR2_USART2SEL_0; // select sysclk for USART2
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;	// enable USART2 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; 	// enable port D clock
#else
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;	// enable USART3 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; 	// enable port D clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;	// enable port B clock
#endif
	
#ifdef STM32F746xx
	// USART2 uses 5 = TX out, 6 = RX in, 3 = CTS in

	GPIOD->AFR[0]  |= ((7UL<<4*3) | (7UL<<4*5) | (7UL<<4*6)); // AF7
	GPIOD->MODER   |= ((2UL<<2*3) | (2UL<<2*5) | (2UL<<2*6)); // alternate mode
	GPIOD->OSPEEDR |= ((2UL<<2*3) | (2UL<<2*5) | (2UL<<2*6)); // fast speed
	GPIOD->PUPDR   |= ((1UL<<2*3) | (1UL<<2*5) | (2UL<<2*6)); // up, up, down

	// USART2 uses PD4 = RTS out

	GPIOD->MODER   |= (1UL<<2*4); 	// general purpose output mode
	GPIOD->OSPEEDR |= (2UL<<2*4); 	// fast speed
	GPIOD->PUPDR   |= (2UL<<2*4); 	// down
	GPIOD->ODR &= ~GPIO_PIN_4; 		// set RTS# low
#else
	// USART3 uses 8 = TX out, 9 = RX in, 11 = CTS in

	GPIOD->AFR[1]  |= ((7UL<<4*(8-8)) | (7UL<<4*(9-8)) |(7UL<<4*(11-8))); // AF7
	GPIOD->MODER   |= ((2UL<<2*8) | (2UL<<2*9) | (2UL<<2*11)); // alternate mode
	GPIOD->OSPEEDR |= ((2UL<<2*8) | (2UL<<2*9) | (2UL<<2*11)); // fast speed
	GPIOD->PUPDR   |= ((1UL<<2*8) | (1UL<<2*9) | (2UL<<2*11)); // up, up, down

	// USART3 uses PB14 = RTS out

	GPIOB->MODER   |= (1UL<<2*14); 	// general purpose output mode
	GPIOB->OSPEEDR |= (2UL<<2*14); 	// fast speed
	GPIOB->PUPDR   |= (2UL<<2*14); 	// down
	GPIOB->ODR &= ~GPIO_PIN_14;     // set RTS# low
#endif

//	typedef struct
//	{
//	  __IO uint32_t CR1;    /*!< USART Control register 1,                 Address offset: 0x00 */
//	  __IO uint32_t CR2;    /*!< USART Control register 2,                 Address offset: 0x04 */
//	  __IO uint32_t CR3;    /*!< USART Control register 3,                 Address offset: 0x08 */
//	  __IO uint32_t BRR;    /*!< USART Baud rate register,                 Address offset: 0x0C */
//	  __IO uint32_t GTPR;   /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
//	  __IO uint32_t RTOR;   /*!< USART Receiver Time Out register,         Address offset: 0x14 */
//	  __IO uint32_t RQR;    /*!< USART Request register,                   Address offset: 0x18 */
//	  __IO uint32_t ISR;    /*!< USART Interrupt and status register,      Address offset: 0x1C */
//	  __IO uint32_t ICR;    /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
//	  __IO uint32_t RDR;    /*!< USART Receive Data register,              Address offset: 0x24 */
//	  __IO uint32_t TDR;    /*!< USART Transmit Data register,             Address offset: 0x28 */
//	} USART_TypeDef;

#ifdef STM32F746xx
	if (sw_rtscts != 0) USART2->CR3 = (1<<11) | (1<<9); // one-bit method, ctse, rtse
	else USART2->CR3 = (1<<11);		// one-bit mode only .. needed for faster baud rates
	USART2->CR2 = (1<<13);   		// two stop bits
	USART2->BRR = (SystemCoreClock)/baudrate; // for using 16 x oversampling

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3); // three bits priority, one bit sub priority
    HAL_NVIC_SetPriority(USART2_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

   	temp = USART2->RDR; // clear the receive buffer
	USART2->CR1 = (1<<5) | (1<<3) | (1<<2) | 1; // over sampling by 16
#else
	//if (sw_rtscts != 0) USART3->CR3 = (1<<11) | (1<<9) | (1<<8); // one-bit method, ctse, rtse
	if (sw_rtscts != 0) USART3->CR3 = (1<<11) | (1<<9); // one-bit method, ctse, rtse
	else USART3->CR3 = (1<<11);		// one-bit mode only .. needed for faster baud rates
	USART3->CR2 = (1<<13);   		// two stop bits
	temp = (((42000000UL<<1)/baudrate)+1)>>1;			// rounding
	USART3->BRR = ((temp&0xFFF8)<<1)|(temp&0x0007); 	// assuming that OVER8 ==1

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3); // three bits priority, one bit sub priority
    HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    
   	temp = USART3->DR; // clear the receive buffer
	USART3->CR1 = (1<<15) | USART_IT_RXNE | (1<<13) | (1<<3) | (1<<2);
#endif

}


//void USART_Send_Buffered(uint8_t);
//void USART_Send_Flush(void);
void USART_Send_Actual(uint8_t value);
//int _write(int file, char *ptr, int len);

// USE THIS
void USART_Send_Actual(uint8_t value)
{
	while (((USART_SH->ISR) & USART_FLAG_TXE) == RESET);
	USART_SH->TDR = value; // send character and clear the TXE flag
}

/*

// Converts 16-bit plot commands to two 8-bit bytes.

void TX_Put(uint16_t value)
{
	USART_Send_Buffered(value>>8);
	USART_Send_Buffered(value&0x00FF);
}

void USART_Send_Actual(uint8_t value)
{
	while (((USART_SH->ISR) & USART_FLAG_TXE) == RESET);
	USART_SH->TDR = value; // send character and clear the TXE flag
}

#define SEND_SIZE 252

char send_buffer[SEND_SIZE+1];
char *send_ptr=&send_buffer[0];
unsigned int send_ctr=0;
unsigned int data_ctr=0;
unsigned int utemp;

// Forms packets that are sent to write when full.
// Presently only used by Tx_Put.
//

void USART_Send_Buffered(uint8_t value)
{
	*send_ptr++ = value;
	send_ctr++;
	if (send_ctr >= SEND_SIZE) {
		USART_Send_Flush();
	} // end packet sent
}

void USART_Send_Flush()
{
	if (send_ctr > 0) _write(3, &send_buffer[0], send_ctr);
	send_ptr = &send_buffer[0];
	send_ctr = 0;
}
*/
int16_t Check_Escape()  // check for escape key strike
{
	if (USART_flag != 0)  {
		if (USART_char == 27) {
			USART_flag = 0;
			return 1;
		}
	}
	return 0;
}

uint16_t USART_Peek()
{
	if (USART_flag != 0) {
		USART_flag = 0;
		return USART_char;
	}
	return (0);
}


//------------------------USART3 RX FIFO read support---------------------------


uint16_t USART_Read(uint8_t *ch)
{
	uint16_t flag;

	GPIOE->BSRR = GPIO_PIN_8;	// use to time USART interrupts
	flag = USART_Fifo_Get(ch);
	if (USART_Fifo_Size() < 3*USART_FIFO_SIZE/4) SET_RTS_HIGH; // actually set RTS# low
	GPIOE->BSRR = GPIO_PIN_8<<16;
	return flag;
}

//------------------------USART_SH interrupt support---------------------------
//
// only have interrupts on received characters

void __attribute__((weak)) USART_SH_ISR_X(void)  // extend USART2 ISR functionality
{
	return;
}

void USART_SH_IRQHandler(void){

	GPIOE->BSRR = GPIO_PIN_8;	// set pin E8 high, used to time USART interrupts

	if ((USART_SH->ISR&USART_IT_RXNE)!=0) {
		USART_char = (USART_SH->RDR)&0x00FF;
		USART_flag = 1;

		if (USART_Fifo_Size() > 3*USART_FIFO_SIZE/4) SET_RTS_LOW; // actually set RTS# high
		else SET_RTS_HIGH;	// actually set RTS# low

		USART_Fifo_Put(USART_char);	// put character into the USART FIFO
	}

	GPIOE->BSRR = GPIO_PIN_8<<16;	// set pin E8 low
}
