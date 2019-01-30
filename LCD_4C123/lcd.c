// LCD.c
// Runs on LM4F120/TM4C123
// Simple device driver for the LCD
// Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Section 4.7.1, Program 4.2 and Program 4.3

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

/*
  size is 1*16
  if do not need to read busy, then you can tie R/W=ground
  ground = pin 1    Vss
  power  = pin 2    Vdd   +3.3V or +5V depending on the device
  ground = pin 3    Vlc   grounded for highest contrast
  PA6    = pin 4    RS    (1 for data, 0 for control/status)
  ground = pin 5    R/W   (1 for read, 0 for write)
  PA7    = pin 6    E     (enable)
  PB0    = pin 7    DB0   (8-bit data)
  PB1    = pin 8    DB1
  PB2    = pin 9    DB2
  PB3    = pin 10   DB3
  PB4    = pin 11   DB4
  PB5    = pin 12   DB5
  PB6    = pin 13   DB6
  PB7    = pin 14   DB7
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/
#include <stdint.h>
#include "LCD.h"
#include "../inc/tm4c123gh6pm.h"
#include "systick.h"

#define E  0x80 // on PA7
#define RS 0x40 // on PA6
#define LCDDATA (*((volatile uint32_t *)0x400053FC))   // PORTB
#define LCDCMD (*((volatile uint32_t *)0x40004300))    // PA7-PA6
#define BusFreq 80            // assuming a 80 MHz bus clock
#define T6us 6*BusFreq        // 6us
#define T40us 40*BusFreq      // 40us
#define T160us 160*BusFreq    // 160us
#define T1600us 1600*BusFreq  // 1.60ms
#define T5ms 5000*BusFreq     // 5ms
#define T15ms 15000*BusFreq   // 15ms
void static OutCmd(unsigned char command){
  LCDDATA = command;
  LCDCMD = 0;           // E=0, R/W=0, RS=0
  SysTick_Wait(T6us);   // wait 6us
  LCDCMD = E;           // E=1, R/W=0, RS=0
  SysTick_Wait(T6us);   // wait 6us
  LCDCMD = 0;           // E=0, R/W=0, RS=0
  SysTick_Wait(T40us);  // wait 40us
}

// Initialize LCD
// Inputs: none
// Outputs: none
void LCD_Init(void){ 
  SYSCTL_RCGC2_R |= 0x00000003;  // 1) activate clock for Ports A and B
  while((SYSCTL_PRGPIO_R&0x03) != 0x03){};// ready?
  GPIO_PORTB_AMSEL_R &= ~0xFF;   // 3) disable analog function on PB7-0
  GPIO_PORTA_AMSEL_R &= ~0xC0;   //    disable analog function on PA7-6              
  GPIO_PORTB_PCTL_R = 0x00000000;   // 4) configure PB7-0 as GPIO   
  GPIO_PORTA_PCTL_R &= ~0xFF000000; //    configure PA7-6 as GPIO
  GPIO_PORTB_DIR_R = 0xFF;       // 5) set direction register
  GPIO_PORTA_DIR_R |= 0xC0;
  GPIO_PORTB_AFSEL_R = 0x00;     // 6) regular port function
  GPIO_PORTA_AFSEL_R &= ~0xC0;
  GPIO_PORTB_DEN_R = 0xFF;       // 7) enable digital port
  GPIO_PORTA_DEN_R |= 0xC0;
  GPIO_PORTB_DR8R_R = 0xFF;      // enable 8 mA drive
  GPIO_PORTA_DR8R_R |= 0xC0;
  SysTick_Init();       // Volume 1 Program 4.7, Volume 2 Program 2.10
  LCDCMD = 0;           // E=0, R/W=0, RS=0
  SysTick_Wait(T15ms);  // Wait >15 ms after power is applied
  OutCmd(0x30);         // command 0x30 = Wake up
  SysTick_Wait(T5ms);   // must wait 5ms, busy flag not available
  OutCmd(0x30);         // command 0x30 = Wake up #2
  SysTick_Wait(T160us); // must wait 160us, busy flag not available
  OutCmd(0x30);         // command 0x30 = Wake up #3
  SysTick_Wait(T160us); // must wait 160us, busy flag not available
  OutCmd(0x38);         // Function set: 8-bit/2-line
  OutCmd(0x10);         // Set cursor
  OutCmd(0x0C);         // Display ON; Cursor ON
  OutCmd(0x06);         // Entry mode set
}
// Output a character to the LCD
// Inputs: letter is ASCII character, 0 to 0x7F
// Outputs: none
void LCD_OutChar(char letter){
  LCDDATA = letter;
  LCDCMD = RS;          // E=0, R/W=0, RS=1
  SysTick_Wait(T6us);   // wait 6us
  LCDCMD = E+RS;        // E=1, R/W=0, RS=1
  SysTick_Wait(T6us);   // wait 6us
  LCDCMD = RS;          // E=0, R/W=0, RS=1
  SysTick_Wait(T40us);  // wait 40us
}

// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear(void){
  OutCmd(0x01);          // Clear Display
  SysTick_Wait(T1600us); // wait 1.6ms
  OutCmd(0x02);          // Cursor to home
  SysTick_Wait(T1600us); // wait 1.6ms
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
