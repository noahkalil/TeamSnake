/*
 * main.c
 *
 *  Created on: Apr 10, 2017
 *      Author: domanski
 */
// Project name: AM Modulation
//
// Implementation of a AM Modulation/Demodulation.
//
// 30Sep2014 .. initial code .. KM
// 11Feb2016 ..                 WS

#include <stdint.h>
#include <stdio.h>
#include <math.h>

void DAC_initialize(uint32_t SampleRate);
void DAC_start(void);
extern volatile int16_t DAC1_value, DAC_ready_flag;
extern volatile int16_t DAC2_value;

void ADC_initialize(uint32_t Sample_Rate);
void ADC_start(void);
extern volatile int16_t ADC1_value, ADC2_value, ADC_ready_flag, ADC_overrun_flag;

void USART_initialize(uint32_t BaudRate, uint8_t cts_rts);

#define PI 3.14159265
#define FS 320000
#define NTABLE 8
#define M 40

float average(int16_t arr[], int size) {
	int i;
	int16_t sum = 0;
	for(i = 0; i < size; ++i) {
		sum += arr[i];
	}
	return sum / size;
}

float max(float arr[], int size) {
	int i;
	float biggest = arr[0];
	for(i = 0; i < size; ++i) {
		if(arr[i] > biggest) biggest = arr[i];
	}
	return biggest;
}

int determine_bit(float arr[], int size) {
	float avg = average(arr, size);
	float biggest = max(arr, size);

	if(avg/biggest > 0.714) {
		return 1;
	} else if(avg/biggest > 0.5) {
		return 0;
	}

	return 2;
}

void print_bitstring(float arr[], int size) {
	int i;
	for(i = 0; i < size; ++i) {
		printf("%i", arr[i]);
	}
	printf('\n');
}

int main(void)
{
	int i, k, query;
	int count = 0;
	int bit_count = 0;
	int location = 0;
	int16_t sine_table[NTABLE];
	int16_t mt;
	int16_t A;
	int32_t zt;
	int16_t st;
	int16_t rt;
	int16_t mhatt;

	int16_t signal[M];
	float bit[1000];
	int bitstring[2] = {0, 0};

	float avg;
	float size;

	USART_initialize(3000000, 1); 	// set baud rate to 3,000,000

	DAC_initialize(FS); 			// set sample rate
	DAC_ready_flag = 0; 		// clear the DAC ready flag
	DAC_start();				// start the DAC running
	ADC_initialize(FS);
	ADC_start();
	ADC_ready_flag = 0; 		// clear the ADC ready flag

	size=(float)NTABLE;
	// create an array of sine function values

	for (i=0; i<NTABLE; i++)
	{
		sine_table[i] = 16000*sin(2.0*PI*i/(float)size);
	}
	k=0;
	A=32768;
	while(1)
	{
		if (ADC_ready_flag !=0 && DAC_ready_flag != 0)
		{
			//  Initial Code
			mt=ADC1_value;
			rt=mt;
			DAC1_value = rt;
			++count;

//====================================================================//
			//    RECEIVER CODE
			//    Input signal from ADC1 (rt)
			//    Output signal to DAC2  (mhatt)

			signal[count] = rt;

			if(count == 39) {
				avg = average(signal, count + 1);
				count = 0;
				bit[bit_count++] = avg;

				if(bit_count == 1000) {
					query = determine_bit(bit, bit_count);
					if(query == 2 && location == 2) {
						print_bitstring(bitstring, NTABLE);
					} else if(query == 1) {
						if(location == 8) location = 0;
						bitstring[location++] = query;
					} else {
						if(location == 8) location = 0;
						bitstring[location++] = query;
					}
				}

				bit_count = bit_count % 1000;
			}



//====================================================================//
			ADC_ready_flag = 0;
			DAC_ready_flag = 0;
			k++;
			k=k%NTABLE;
		}
	}



}



