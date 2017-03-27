// File name: syscalls_1_3.c
//
// 2014      .. used in ENGIN 100 Fall 2014 .. from CooCox?
// 03May2016 .. updated for read/write with WindowPlot .. KM
// 22Sep2016 .. combined SDID and SOH flag into one byte .. KM
// 23Sep2016 .. made packet support separate module .. KM

/**************************************************************************//*****
 * @file     stdio.c
 * @brief    Implementation of newlib syscall
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "WindowPlot.h"

// **** Functions to communicate with remote system

void USART_Send_Actual(uint16_t value);
uint16_t USART_Read(uint8_t *ch);
uint16_t GetPacket(uint16_t *sid, uint8_t *packet_buffer, uint16_t *len);
int _write(int fid, char *ptr, int len);
uint16_t SendPacket(uint16_t, uint8_t *, uint16_t);


void SendToRemote(uint8_t value)
{
    USART_Send_Actual(value);  // use uint8_t????
}

uint16_t ReceiveFromRemote(uint8_t *ch)
{
   return USART_Read(ch);
}

void mySleep(uint16_t duration)
{
    //not used here
}

void SendToPlot(uint16_t value)
{
    while(1); // not supported at this end
}

// **** End of remote access functions

//__attribute__ ((used))
//int link(char *old, char *new) {
//return -1;
//}


char command_string[128];
char USART_input_buffer[256];

__attribute__ ((used))
int _open(char* file, int flags, int mode)
{
	sprintf(command_string, "open %s %X %X", file, flags, mode);
	_write(4, command_string, strlen(command_string)+1); // send ending 0 also
	return 16;
}

__attribute__ ((used))
int _close(int file)
{
	sprintf(command_string, "close 0x%X", file);
	_write(4, command_string, strlen(command_string)+1);
  return -1;
}

__attribute__ ((used))
int _fstat(int file, struct stat *st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

__attribute__ ((used))
int _isatty(int file)
{
  return 1;
}

__attribute__ ((used))
int _lseek(int file, int ptr, int dir)
{
  return 0;
}

//************************************


uint8_t *buffer_ptr, value;
unsigned short sw, partial, flag;

__attribute__ ((used))
int _read(int sid, char *ptr, int len)
{
	uint16_t returned_sid=sid, local_len;

	sprintf(command_string, "read: %X %X", sid, len);
	_write(4, command_string, strlen(command_string)+1); // send ending 0 also
	GetPacket(&returned_sid, (uint8_t *)ptr, &local_len);  // read packet from remote
	return local_len;
}

//************************************


__attribute__ ((used))
int _write(int sid, char *ptr, int len)
{
    SendPacket((uint16_t)sid, (uint8_t *)ptr, (uint16_t)len);
    return len;
}

__attribute__ ((used))
void abort(void)
{
  /* Abort called */
  while(1);
}

         
/* --------------------------------- End Of File ------------------------------ */
