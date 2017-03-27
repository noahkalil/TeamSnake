/* File name: Support_Plotting_1_2.c

    03Dec2002 .. initial Tektronix 4010 version started .. K.Metzger
    22Feb2004 .. updated for EECS 452 .. K.Metzger
    17May2007 .. FPGA specific protocol implemented .. K.Metzger
	05May2014 .. updated for use with STM32F4 Discovery .. K.Metzger
	17Jul2014 .. renamed and plot frame drawing support moved into file .. K.Metzger
	26Jan2017 .. replaced flush call to TX_Flush

   The primitive operations derive from pen plotter routines.  penup moves
   the plotting "pen" to a given position in the up (non-printing) position.
   pendn does the same but with the pen in contact with the drawing surface.
   pendot moves pen up and then puts the pen down.  This is like doing a
   penup placing the pen in contact with the drawing surface at the end.

   The display coordinate system has origin in lower left corner of the
   screen.  The full x extent is 1024 points.  The full y extent is 768
   visible points.  These routines work in "raw" coordinates.

*/

//#pragma optimize_level=3

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "PlotSupport.h"
#include "WindowPlot.h"

// Define the plot area size in pixels

#define XS PLT_X_SIZE
#define YS PLT_Y_SIZE

void TX_Flush(void);

static unsigned short int current_color = 0x00FF;

void InitPlotSupport(uint16_t SW_DrawPlotPageBox)
{
	StartPlot();
	current_color = PLT_BLACK;
	if (SW_DrawPlotPageBox != 0) {
		GoTo(PLT_RED, 0, 0);			DrawTo(XS-1, 0);
		GoTo(PLT_BLUE, XS-1, 0);		DrawTo(XS-1, YS-1);
		GoTo(PLT_GREEN, XS-1, YS-1); 	DrawTo(0, YS-1);
		GoTo(PLT_BLACK, 0, YS-1); 		DrawTo(0, 0);
	}
    return;
}   

void StartPlot()
{
	TX_Put(0); TX_Put(0); // have to flush something somewhere
	TX_Put(PLT_STARTPLOT);
}

void UpdatePlot()
{
	TX_Put(PLT_UPDATEPLOT);
	TX_Flush();
	SetCurrentColor(current_color);

}

void MarkSet()
{
	TX_Put(PLT_MARK_SET);
}

void MarkClear()
{
	TX_Put(PLT_MARK_CLEAR);
}

void ReturnToMark()
{
	TX_Put(PLT_RETURN_TO_MARK);
}

void EndPlot()
{
	TX_Put(PLT_ENDPLOT);
	TX_Flush();
}

void SetCurrentColor(int16_t color) {
	current_color = color;
	TX_Put(PLT_SELECTCOLOR|(color&0x0007));
}

void GoTo(int16_t color, int16_t x, int16_t y)
{
	if (color != current_color) {
		TX_Put(PLT_SELECTCOLOR|(color&0x0007));
		current_color = color;
	}
	TX_Put(PLT_MOVETO_X|(x&0x03FF));
	TX_Put(PLT_MOVETO_Y|(y&0x03FF));
    return;
}

void DrawTo(int16_t x, int16_t y)
{      
	TX_Put(PLT_DRAWTO_X|(x&0x03FF));
	TX_Put(PLT_DRAWTO_Y|(y&0x03FF));
    return;
}

void PrintString(int16_t x, int16_t y, int16_t step, int16_t rotation, int16_t color, char *cp)
{
	char ch;

	GoTo(color, x, y);
	TX_Put(PLT_CHARPROPERTIES | ((step&0x000F)<<5)| ((rotation&0x0003)<<3)| (color&0x0007));
	while((ch=*cp++) != '\0') {
		TX_Put(PLT_DRAWCHAR|(ch&0x007F));
	}
}

// Support for data coordinate drawing

struct PLOT {
    float Pxs;        // plot x start        .. sheet fraction
    float Pxlen;      // plot x axis length  .. sheet fraction
    float Pys;        // plot y origin       .. sheet fraction
    float Pylen;      // plot y axis length  .. sheet fraction
    float xs;         // x real world start value
    float xr;         // x real world range value
    float ys;         // y real world start value
    float yr;         // y real world range value
    float x_screen;
    float y_screen;
} plot[4]; // four plots supported


void GoToSheet(int16_t color, float Sx, float Sy)
{
    GoTo(color, (int16_t)(Sx*XS+0.5), (int16_t)(Sy*YS+0.5));
}

void DrawToSheet(float Sx, float Sy)
{
    DrawTo((int16_t)(Sx*XS+0.5), (int16_t)(Sy*YS+0.5));
}

void GoToData(int16_t f_n, int16_t color, float Px, float Py)
{
    float fraction, Fx, Fy;
    struct PLOT *p;

    p = &plot[f_n];
    fraction = (Px - p->xs)/p->xr;
    Fx = p->Pxs+fraction*p->Pxlen;
    fraction = (Py-p -> ys)/p->yr;
    Fy = p->Pys+fraction*p->Pylen;
    GoToSheet(color, Fx, Fy);
    p->x_screen = Fx;
    p->y_screen = Fy;
}

void DrawToData(int16_t f_n, int16_t color, float Px, float Py)
{
    float fraction, Fx, Fy;
    struct PLOT *p;

    p = &plot[f_n];
    GoToSheet(color, p->x_screen, p->y_screen);

    fraction = (Px - p->xs)/p->xr;
    Fx = p->Pxs+fraction*p->Pxlen;
    fraction = (Py - p->ys)/p->yr;
    Fy = p->Pys+fraction*p->Pylen;
    DrawToSheet(Fx, Fy);
    p->x_screen = Fx;
    p->y_screen = Fy;
}

void PrintStringData(float x, float y, int16_t step, int16_t rotation, int16_t color, char *cp)
{
	char ch;

	GoToSheet(color, x, y);
	TX_Put(PLT_CHARPROPERTIES | (step&0x000F)<<5| ((rotation&0x0003)<<3) | (color&0x0007));
	while((ch=*cp++) != '\0') {
		TX_Put(PLT_DRAWCHAR | (ch&0x00FF));
	}
}

void Y_AxisGridLine(int16_t p_n, int16_t color, float xloc)
{
    struct PLOT *p;

	p = &plot[p_n];
    GoToData(p_n, color, xloc, p->ys);
    DrawToData(p_n, color, xloc, p->ys + p->yr);
}

void Y_AxisLabel(int16_t p_n, char * label, int16_t offset)
{
    float cdx, cdy, x, y, n, step = 3;
    struct PLOT *p;

    p = &plot[p_n];
    cdx = 3.0*step/XS; cdy = 5.5*step/YS;
    n = strlen(label);

    x = p->Pxs - offset*cdx - cdy;
    y = p->Pys + p->Pylen/2 - (n+1)*cdx/2;

   PrintStringData(x, y, step, 1, 3, label);
}

void X_AxisTic(int16_t p_n, int16_t color, float xloc, char *string)
{
    float cdx, cdy, x, y, n, step = 3;
    struct PLOT *p;

    p = &plot[p_n];
    cdx = 3.0*step/XS; cdy = 5.5*step/YS;
    n = strlen(string);

    x = p->Pxs + p->Pxlen*(xloc - p->xs)/p->xr;
    y = p->Pys;

    GoToSheet(color, x, y);
    DrawToSheet(x, y-cdy/2);

    PrintStringData(x-(n+1)*cdx/2, y-2*cdy, step, 0, 3, string);
}

void DrawHorizontalGridLine(int16_t p_n, int16_t color, float value, char *string)
{
    float cdx, cdy, x, y, n, step = 3;
    struct PLOT *p;

    p = &plot[p_n];
    cdx = 3.0*step/XS; cdy = 5.5*step/YS;
    x = p->Pxs;
    y = p->Pys + p->Pylen*(value - p->ys)/p->yr;
    GoToSheet(color, x-cdx, y);
    x = p->Pxs + p->Pxlen;
    DrawToSheet(x, y);
    n = strlen(string)+2;
    PrintStringData(p->Pxs - n*cdx, y-cdy/2, step, 0, 3, string);
}

void X_AxisLabel(int16_t p_n, char * label)
{
    float cdx, cdy, x, y, n, step = 3;
    struct PLOT *p;

    p = &plot[p_n];
    cdx = 3.0*step/XS; cdy = 5.5*step/YS;
    n = strlen(label);

    x = p->Pxs + p->Pxlen/2;
    y = p->Pys;

    PrintStringData(x-(n+1)*cdx/2, y-3.5*cdy, step, 0, 3, label);
}
