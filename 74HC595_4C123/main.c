// main.c
// Runs on TM4C123
// Use SSI0 to send a 8-bit code to the 74HC595.
// Output port expander
// If running at 80 MHz change SSI0_CPSR_R to 4
// Daniel Valvano
// July 17, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 7.4

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
// TM4C123      74HC595
//   +3.3       pin 16 Vcc powewr
//   Gnd        pin 8  ground
//   Gnd        pin 13 G*
//  +3.3        pin 10 SCLR*
// PA2 SSI0Clk  pin 11 SCK
// PA3 SSI0Fss  pin 12 RCK
// PA5 SSI0Tx   pin 14 SER

// Port         74HC595
// bit 7 (msb)  pin 7  Qh
// bit 6        pin 6  Qg
// bit 5        pin 5  Qf
// bit 4        pin 4  Qe
// bit 3        pin 3  Qd
// bit 2        pin 2  Qc
// bit 1        pin 1  Qb
// bit 0 (LSB)  pin 15 Qa

// see Figure 7.19 for complete schematic
#include <stdint.h>
#include "74HC595.h"
#include "SysTick.h"

uint8_t Data=0;
int main1(void) { // 16 MHz
  Port_Init();
  while(1){
    Port_Out(Data);
    Data++;
  }
}


// 4-bit solution, LCD using 3 pins instead of 10.
/*
  size is 1*16
  tie R/W=ground because shift register is output only
      ground   = LCD pin 1    Vss
      power    = LCD pin 2    Vdd   +3.3V or +5V depending on the device
      ground   = LCD pin 3    Vlc   grounded for highest contrast
  595 Qf 5     = LCD pin 4    RS    (1 for data, 0 for control/status)
      ground   = LCD pin 5    R/W   (1 for read, 0 for write)
  595 Qg 6     = LCD pin 6    E     (enable)
  595 Qa 15    = LCD pin 11   DB4   (4-bit data)
  595 Qb 1     = LCD pin 12   DB5
  595 Qc 2     = LCD pin 13   DB6
  595 Qd 3     = LCD pin 14   DB7
  595 Qe 4     = nc (if friendly, could be used)
  595 Qh 7     = nc (if friendly, could be used)
      nc       = LCD pin 7    DB0
      nc       = LCD pin 8    DB1
      nc       = LCD pin 9    DB2
      nc       = LCD pin 10   DB3
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/
#define E 0x40
#define RS 0x20
#define BusFreq 16            // assuming a 16 MHz bus clock
#define T6us 6*BusFreq        // 6us
#define T40us 40*BusFreq      // 40us
#define T160us 160*BusFreq    // 160us
#define T1600us 1600*BusFreq  // 1.60ms
#define T5ms 5000*BusFreq     // 5ms
#define T15ms 15000*BusFreq   // 15ms
//---------------------outCsrNibble---------------------
// sends one command code to the LCD control/status
// Input: command is 4-bit function to execute
// Output: none
static void outCsrNibble(uint8_t command){
  Port_Out(command&0x0F);      // nibble, E=0, RS=0
  Port_Out(E|(command&0x0F));  // E goes 0,1
  SysTick_Wait(T6us);          // wait 6us
  Port_Out(command&0x0F);      // E goes 1,0
  SysTick_Wait(T6us);          // wait 6us
}

//---------------------OutCmd---------------------
// sends one command code to the LCD control/status
// Input: command is 8-bit function to execute
// Output: none
static void OutCmd(unsigned char command){
  outCsrNibble(command>>4);   // ms nibble, E=0, RS=0
  outCsrNibble(command);      // ls nibble, E=0, RS=0
  SysTick_Wait(T40us);        // wait 40us
}

//---------------------LCD_Clear2---------------------
// clear the LCD display, send cursor to home
// Input: none
// Output: none
void LCD_Clear(void){
  OutCmd(0x01);          // Clear Display
  SysTick_Wait(T1600us); // wait 1.6ms
  OutCmd(0x02);          // Cursor to home
  SysTick_Wait(T1600us); // wait 1.6ms
}
#define LCDINC 2
#define LCDDEC 0
#define LCDSHIFT 1
#define LCDNOSHIFT 0
#define LCDCURSOR 2
#define LCDNOCURSOR 0
#define LCDBLINK 1
#define LCDNOBLINK 0
#define LCDSCROLL 8
#define LCDNOSCROLL 0
#define LCDLEFT 0
#define LCDRIGHT 4
#define LCD2LINE 8
#define LCD1LINE 0
#define LCD10DOT 4
#define LCD7DOT 0

//---------------------LCD_Init2---------------------
// initialize the LCD display, called once at beginning
// Input: none
// Output: none
void LCD_Init(void){   // 16 MHz
  SysTick_Init();        // Volume 1 Program 4.7, Volume 2 Program 2.10
  Port_Init();           // initialize 74HC595 interface using eUSCI3
  Port_Out(0);           // E=0
  SysTick_Wait(T15ms);   // Wait >15 ms after power is applied
  outCsrNibble(0x03);    // (DL=1 8-bit mode)
  SysTick_Wait(T5ms);    // must wait 5ms, busy flag not available
  outCsrNibble(0x03);    // (DL=1 8-bit mode)
  SysTick_Wait(T160us);  // must wait 160us, busy flag not available
  outCsrNibble(0x03);    // (DL=1 8-bit mode)
  SysTick_Wait(T160us);  // must wait 160us, busy flag not available
  outCsrNibble(0x02);    // (DL=0 4-bit mode)
  SysTick_Wait(T160us);  // must wait 160us, busy flag not available
/* Entry Mode Set 0,0,0,0,0,1,I/D,S
     I/D=1 for increment cursor move direction
        =0 for decrement cursor move direction
     S  =1 for display shift
        =0 for no display shift   */
  OutCmd(0x04+LCDINC+LCDNOSHIFT);        // I/D=1 Increment, S=0 no displayshift
/* Display On/Off Control 0,0,0,0,1,D,C,B
     D  =1 for display on
        =0 for display off
     C  =1 for cursor on
        =0 for cursor off
     B  =1 for blink of cursor position character
        =0 for no blink   */
  OutCmd(0x0C+LCDNOCURSOR+LCDNOBLINK);   // D=1 displayon, C=0 cursoroff, B=0 blink off
/* Cursor/Display Shift  0,0,0,1,S/C,R/L,*,*
     S/C=1 for display shift
        =0 for cursor movement
     R/L=1 for shift to left
        =0 for shift to right   */
  OutCmd(0x10+LCDNOSCROLL+LCDRIGHT);   // S/C=0 cursormove, R/L=1 shiftright
/* Function Set   0,0,1,DL,N,F,*,*
     DL=1 for 8 bit
       =0 for 4 bit
     N =1 for 2 lines
       =0 for 1 line
     F =1 for 5 by 10 dots
       =0 for 5 by 7 dots */
  OutCmd(0x20+LCD2LINE+LCD7DOT);   // DL=0 4bit, N=1 2 line, F=0 5by7 dots
}

//---------------------LCD_OutChar---------------------
// sends one ASCII to the LCD display
// Input: letter is ASCII code
// Output: true if successful
void LCD_OutChar(unsigned char letter){
  Port_Out(RS|(0x0F&(letter>>4)));     // ms nibble, E=0, RS=1
  Port_Out(E|RS|(0x0F&(letter>>4)));   // E goes 0 to 1
  SysTick_Wait(T6us);                  // wait 6us
  Port_Out(RS|(0x0F&(letter>>4)));     // E goes 1 to 0
  SysTick_Wait(T6us);                  // wait 6us
  Port_Out(RS|(0x0F&letter));          // ls nibble, E=0, RS=1
  Port_Out(E|RS|(0x0F&letter));        // E goes 0 to 1
  SysTick_Wait(T6us);                  // wait 6us
  Port_Out(RS|(0x0F&letter));          // E goes 1 to 0
  SysTick_Wait(T40us);                 // wait 40us
}

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString(char *pt){
  while(*pt){
    LCD_OutChar(*pt);
    pt++;
  }
}

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    LCD_OutUDec(n/10);
    n = n%10;
  }
  LCD_OutChar(n+'0'); /* n is between 0 and 9 */
}

//--------------------------LCD_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    LCD_OutUHex(number/0x10);
    LCD_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      LCD_OutChar(number+'0');
     }
    else{
      LCD_OutChar((number-0x0A)+'A');
    }
  }
}

//-----------------------LCD_GoTo-----------------------
// Move cursor
// Input: line number is 1 to 4, column from 1 to 20
// Output: none
// errors: it will check for legal address
void LCD_GoTo(int line, int column){
  unsigned char DDaddr;
  if((line<1) || (line>4)) return;
  if((column<1) || (column>20)) return;
  if(line==1) DDaddr = column-1;
  if(line==2) DDaddr = 0x3F+column;
  if(line==3) DDaddr = 0x13+column;
  if(line==4) DDaddr = 0x53+column;
  DDaddr += 0x80;
  OutCmd(DDaddr);
}

int main(void){  uint32_t n;
  LCD_Init();  // leave system clock at 16 MHz, initialize LCD, initialize SysTick, and initialize 74HC595 interface
  n = 0;
  LCD_Clear();
  LCD_OutString("Test LCD");
  SysTick_Wait10ms(100);
  while(1){
    LCD_GoTo(2, 1);
    LCD_OutUDec(n);
    SysTick_Wait10ms(10);
    LCD_OutString(",  0x");
    LCD_OutUHex(n);
    LCD_OutString("      ");
    SysTick_Wait10ms(100);
    n++;
  }
}
