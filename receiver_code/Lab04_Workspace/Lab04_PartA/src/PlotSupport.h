// File name: PlotSupport.h
//
// 8Feb 2012 .. original version .. K.Metzger
// 05May2014 .. updated for STM32F4 Discovery .. KM
// 05Mar2016 .. updated to make consistent for documentation .. KM
// 29Jun2016 .. updated .. KM

#ifndef PLOT_SUPPORT_H_
#define PLOT_SUPPORT_H_

#include <stdint.h>

#define PLT_X_SIZE 1024
#define PLT_Y_SIZE 768

// Configuration and control functions

void InitPlotSupport(uint16_t);
void StartPlot();
void UpdatePlot();
void MarkSet();
void MarkClear();
void ReturnToMark();
void EndPlot(void);
void SetCurrentColor(int16_t color);

void TX_Put(uint16_t);	// typically defined in USART support

// Window coordinate plotting functions

void GoTo(int16_t color, int16_t x, int16_t y);
void DrawTo(int16_t x, int16_t y);
void PrintString(int16_t x, int16_t y, int16_t step, int16_t rotation, int16_t color, char *cp);

// Data coordinate plotting functions

void GoToData(int16_t f_n, int16_t color, float Px, float Py);
void DrawToData(int16_t f_n, int16_t color, float Px, float Py);
void PrintStringData(float x, float y, int16_t step, int16_t rotation, int16_t color, char *cp);

void DrawHorizontalGridLine(int16_t p_n, int16_t color, float value, char *string);
void X_AxisTic(int16_t p_n, int16_t color, float xloc, char *string);
void X_AxisLabel(int16_t p_n, char * label);
void Y_AxisLabel(int16_t p_n, char * label, int16_t offset);

#endif /* FPGA_PLOT_H_ */
