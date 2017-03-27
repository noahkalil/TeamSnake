// File name: WindowPlot.h
//
//  Created on:  May 20, 2015 .. K.Metzger
//  Current version: May 18, 2016 .. KM
//  

#ifndef WINDOWPLOT_H_
#define WINDOWPLOT_H_

#define PLT_NULL				(0<<11)
#define PLT_MOVETO_X  			(1<<11)
#define PLT_MOVETO_Y			(2<<11)
#define PLT_DRAWTO_X 			(3<<11)
#define PLT_DRAWTO_Y			(4<<11)
#define PLT_CHARPROPERTIES		(5<<11)
#define PLT_DRAWCHAR 			(6<<11)
#define PLT_SELECTCOLOR			(7<<11)

#define PLT_DOSOMETHING  		(8<<11)
	#define PLT_STARTPLOT		(PLT_DOSOMETHING|1)
	#define PLT_ENDPLOT	 		(PLT_DOSOMETHING|2)
	#define PLT_UPDATEPLOT 		(PLT_DOSOMETHING|3)
	#define PLT_MARK_SET		(PLT_DOSOMETHING|4)
	#define PLT_MARK_CLEAR		(PLT_DOSOMETHING|5)
	#define PLT_RETURN_TO_MARK	(PLT_DOSOMETHING|6)

#define 	PLT_BLACK	(0x00)
#define 	PLT_RED		(0x01)
#define 	PLT_GREEN	(0x02)
#define		PLT_BLUE	(0x03)
#define		PLT_YELLOW	(0x04)
#define		PLT_CYAN	(0x05)
#define		PLT_MAGENTA	(0x06)
#define		PLT_WHITE   (0x07)

// character properties:
//
//    ((step&0x000F)<<5)|((rotation&0x0003)<<3)|(color&0x0007))
//
//	Defines for the WindowPlot executable and client use.

#define WP_SOH 0xA5
#define SID_STDIN 0
#define SID_STDOUT 1
#define SID_STDERR 2
#define SID_WINDOWPLOT 3
#define SID_IO_CALL 4
#define SID_FROM_WHATEVER 5
#define SID_TO_WHATEVER 6

#endif /* WINDOWPLOT_H_ */
