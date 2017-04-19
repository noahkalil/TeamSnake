// Project name: WWVB Modulation
//
// Implementation of a WWVB Modulation
//
// 30Sep2014 .. initial code .. KM
//  11Feb 2016 ..            .. WS

#include <stdint.h>
#include <stdio.h>
#include <math.h>


void DAC_initialize(uint32_t SampleRate);
void DAC_start(void);
extern volatile int16_t DAC1_value, DAC_ready_flag;
//extern volatile int16_t DAC2_value;

void USART_initialize(uint32_t BaudRate, uint8_t cts_rts);
void PINS_Initialize();

#define PI 3.14159265
#define FS 320000
#define NTABLE 6



int main(void)
{
	int i,isec,j,k,data;
	int sine_table[NTABLE];
	int yt,fc;
	float size;
	float t;
	int st0,st1,st2;

	USART_initialize(3000000, 1); 	// set baud rate to 3,000,000
	Pins_initialize();
	DAC_initialize(FS); 			// set sample rate
	DAC_ready_flag = 0; 		// clear the DAC ready flag
	DAC_start();				// start the DAC running

	size=NTABLE;
	fc=40000;
	// create an array of sine function values

	for (i=0; i<NTABLE; i++)
	{
		sine_table[i] = 4681*sin(2.0*PI*i/(float)size)*cos(2.0*PI*i/(float)size);
		// 4681=32767/7
	}

	while(1)
	{
		for (isec=0; isec<60; isec++)
		{
			LED_Blue_Off();
			LED_Green_Off();
			LED_Red_Off();
			if (isec%2==0) LED_Orange_On();
			else LED_Orange_Off();
			if (isec%10==9 | isec==0)
			{
				data=2;
				LED_Blue_On();
			}
			else
			{
				data=rand() %2;
				if (data==0) LED_Red_On();
				else LED_Green_On();
			}


			//printf("data( %i) = %i \n",isec,data);
			for (j=0; j<fc; j++)
			{
				t=(float)j/((float)fc);
				if (t<0.2) st0=1; else st0=7;
				if (t<0.5) st1=1; else st1=7;
				if (t<0.8) st2=1; else st2=7;

				k=0;
				while(k<NTABLE)
				{
					switch(data)
						{
							case 0: yt=st0*sine_table[k]; break;
							case 1: yt=st1*sine_table[k]; break;
							case 2: yt=st2*sine_table[k]; break;
						}

					if (DAC_ready_flag != 0)
					{
						DAC1_value = yt;
						DAC_ready_flag = 0;
						k++;
					}
				}
			}
		}
	}
}

