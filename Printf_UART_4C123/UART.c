// UART.c
// Runs on LM4F120, TM4C123
// Simple device driver for the UART.
// Daniel Valvano
// May 5, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
 
 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1

#include <stdio.h>
#include <stdint.h>
#include "UART.h"
#include "../inc/tm4c123gh6pm.h"

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void UART_Init(void){
  SYSCTL_RCGCUART_R |= 0x01;            // activate UART0
  SYSCTL_RCGCGPIO_R |= 0x01;            // activate port A
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 8;                     // IBRD = int(16,000,000 / (16 * 115,200)) = int(8.680)
  UART0_FBRD_R = 44;                    // FBRD = round(0.5104 * 64 ) = 33
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8|UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
                                        // configure PA1-0 as UART
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}

//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART_InChar(void){
  while((UART0_FR_R&UART_FR_RXFE) != 0);
  return((char)(UART0_DR_R&0xFF));
}
//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART_OutChar(char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}

// Print a character to UART.
int fputc(int ch, FILE *f){
  if((ch == 10) || (ch == 13) || (ch == 27)){
    UART_OutChar(13);
    UART_OutChar(10);
    return 1;
  }
  UART_OutChar(ch);
  return 1;
}
// Get input from UART, echo
int fgetc (FILE *f){
  char ch = UART_InChar();  // receive from keyboard
  UART_OutChar(ch);            // echo
  return ch;
}
// Function called when file error occurs.
int ferror(FILE *f){
  /* Your implementation of ferror */
  return EOF;
}

// Abstraction of general output device
// Volume 2 section 3.4.5


// Clear display
void Output_Clear(void){ // Clears the display
  // not implemented on the UART
}
// Turn off display (low power)
void Output_Off(void){   // Turns off the display
  // not implemented on the UART
}
// Turn on display
void Output_On(void){    // Turns on the display
  // not implemented on the UART
}
// set the color for future output
void Output_Color(uint32_t newColor){ // Set color of future output 
  // not implemented on the UART
}


#ifdef __TI_COMPILER_VERSION__
	//Code Composer Studio Code
#include "file.h"
int uart_open(const char *path, unsigned flags, int llv_fd){
  UART_Init();
  return 0;
}
int uart_close( int dev_fd){
	return 0;
}
int uart_read(int dev_fd, char *buf, unsigned count){char ch;
  ch = UART_InChar();    // receive from keyboard
  ch = *buf;         // return by reference
  UART_OutChar(ch);  // echo
  return 1;
}
int uart_write(int dev_fd, const char *buf, unsigned count){ unsigned int num=count;
	while(num){
		UART_OutChar(*buf);
		buf++;
		num--;
	}
	return count;
}
off_t uart_lseek(int dev_fd, off_t ioffset, int origin){
	return 0;
}
int uart_unlink(const char * path){
	return 0;
}
int uart_rename(const char *old_name, const char *new_name){
	return 0;
}

//------------Output_Init------------
// Initialize the UART for 115,200 baud rate (assuming 3 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void Output_Init(void){int ret_val; FILE *fptr;
  UART_Init();
  ret_val = add_device("uart", _SSA, uart_open, uart_close, uart_read, uart_write, uart_lseek, uart_unlink, uart_rename);
  if(ret_val) return; // error
  fptr = fopen("uart","w");
  if(fptr == 0) return; // error
  freopen("uart:", "w", stdout); // redirect stdout to uart
  setvbuf(stdout, NULL, _IONBF, 0); // turn off buffering for stdout

}
#else
//Keil uVision Code
//------------Output_Init------------
// Initialize the Nokia5110
// Input: none
// Output: none
//------------Output_Init------------
// Initialize the UART for 115,200 baud rate (assuming 16 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none
void Output_Init(void){
  UART_Init();
}
#endif
