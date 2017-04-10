// Project name: DAC_DDS
//
// Implementation of a Direct Digital Synthesizer.
// The update clock frequency is set using a #define.
// User enters desired output frequency.  Program
// computes the needed frequency tuning value and
// generates a sine wave at that frequency.
//
// 30Sep2014 .. initial code .. KM

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "diag/Trace.h"
void USART_initialize(uint32_t BaudRate, uint8_t cts_rts);

unsigned int int_to_int(unsigned int k){
	return (k == 0 || k == 1 ? k : ((k % 2) + 10 * int_to_int(k/2)));
}

int main(void)
{
	USART_initialize(9600, 1); 	// set baud rate to 3,000,000
	unsigned int right = int_to_int(0x4F);
	unsigned int left = int_to_int(0x50);
	unsigned int down = int_to_int(0x51);
	unsigned int up = int_to_int(0x52);
	unsigned long wait = 20000000, i = 0;

	for(;;) {
		printf("%.8d\n",right);
		for(i = 0; i < wait; ++i) {}

		printf("%.8d\n",down);
		for(i = 0; i < wait; ++i) {}

		printf("%.8d\n",left);
		for(i = 0; i < wait; ++i) {}

		printf("%.8d\n",up);
		for(i = 0; i < wait; ++i) {}
	}

}



