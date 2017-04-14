//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

// Standalone STM32F4 empty sample (trace via DEBUG).
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_gpio.h"
void USART_initialize(uint32_t baudrate, uint8_t sw_rtscts);
void Pins_initialize();

char* num2button(int num, char* str) {
  if (num == -1)
    str = "NONE";

  else if (num == 0)
    str = "UP";

  else if (num == 1)
    str = "DOWN";

  else if (num == 2)
    str = "LEFT";

  else if (num == 3)
    str = "RIGHT";

  return str;
}

int main(int argc, char** argv) {
  int   button = 0;
  char  but_str[8];

  // At this stage the system clock should have already been configured
  // at high speed.
	USART_initialize(3000000, 0);
	Pins_initialize();

  // Infinite loop
  for (;;) {
    // wait a little
	  for (i=0; i < 10000000; i++);
    button = Button_pressed();    // read a button
    num2button(button, but_str);  // save string button to but_str

    // read in button pressed. up, down, left, right is [0..3]
	  printf("Read button: %d (aka %s)\n", button, but_str);
  }

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
