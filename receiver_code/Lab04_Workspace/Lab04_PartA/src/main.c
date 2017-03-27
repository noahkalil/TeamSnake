// Project name: Snake

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "diag/Trace.h"
void USART_initialize(uint32_t BaudRate, uint8_t cts_rts);

int main(void) {
	USART_initialize(9600, 1); 	// set baud rate to 9600 for /dev/ttyUSB0
	while(1) {
		printf("Hello, World!\n");   //  THIS PRINTS TO WINDOWPLOT/USART
		
    //trace_printf("i= %i \n",i);   //  THIS PRINTS TO DEBUGGER CONSOLE
	}
}



