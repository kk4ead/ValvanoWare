// WS2812.h
// Runs on TM4C123
// Provide functions to interface with a WS2812 smart LED driver.
// This device takes a square wave encoded with 24 bits of color
// and sets its LED to the desired color.  Subsequent waves are
// then passed on to the next LEDs in the chain.
// Daniel Valvano
// June 24, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

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

// WS2812 data connected to PE0 through 300 to 500 ohm resistor and level shifter circuit
// 1,000 uF, 6.3 V capacitor across power rails

#define USEDMA 1    // 0 for SysTick timing; non-zero for DMA Channel 8 and Timer 5A
#define NUMCOLS 36  // number of LEDs per string
#define NUMROWS 3   // number of LED strings (1<=NUMROWS<=8); strings connected to the least-significant bits of Port E
#define ROWMASK  ((1<<NUMROWS) - 1)                       // bit mask for all rows for direction, alternate function, etc.
#define ROWMASK2 ((0xFFFFFFFF>>(4*NUMROWS))<<(4*NUMROWS)) // bit mask for all rows for port control

//------------WS2812_Init------------
// Initialize all peripherals needed to interface with a WS2812.
// Set system clock to 80 MHz using the PLL and initialize
// either the SysTick timer without interrupts or DMA channel 8
// with Timer5A.  Choose the transfer mode at compile time using
// "USEDMA" in WS2812.h.  Configure PE as GPIO out.
// Input: none
// Output: none
void WS2812_Init(void);

//------------WS2812_SendReset------------
// Send a reset command to all WS2812 LED drivers connected
// to PE.  This function uses a blind wait, so it takes at
// least 50 usec to complete.
// Input: none
// Output: none
// Assumes: 80 MHz system clock, PE initialized
void WS2812_SendReset(void);

//------------WS2812_ClearBuffer------------
// Clear the entire RAM buffer and restart all cursors to
// the beginning of the row of LEDs.  To actually update the
// physical LEDs, call WS2812_PrintBuffer().
// Input: none
// Output: none
void WS2812_ClearBuffer(void);

//------------WS2812_AddColor------------
// Configure the next LED with the desired color values and
// move to the next LED automatically.  This updates the RAM
// buffer and increments the cursor.  To actually update the
// physical LEDs, call WS2812_PrintBuffer().  A color value
// of zero means that color is not illuminated.
// Input: red   8-bit red color value
//        green 8-bit green color value
//        blue  8-bit blue color value
//        row   index of LED row (0<=row<NUMROWS)
// Output: none
void WS2812_AddColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t row);

//------------WS2812_SetCursor------------
// Point the cursor at the desired LED index.  This LED will
// be the next one configured by the next call to
// WS2812_AddColor().  If the new cursor parameter is greater
// than the total number of LEDs, it will wrap around.  For
// example, WS2812_SetCursor(76, 0) with a row of 75 LEDs
// will wrap around to the second (index=1) LED for the first
// LED string.
// Input: newCursor 8-bit new cursor index
//        row       index of LED row (0<=row<NUMROWS)
// Output: none
void WS2812_SetCursor(uint8_t newCursor, uint8_t row);

//------------WS2812_GetCursor------------
// Return the current cursor index.  This is the index of the
// LED that will be the next one configured by the next call
// to WS2812_AddColor().
// Input: row  index of LED row (0<=row<NUMROWS)
// Output: 8-bit current cursor index
uint8_t WS2812_GetCursor(uint8_t row);

//------------WS2812_PrintBuffer------------
// Send all 24-bit color values to the chain of WS2812 LED
// drivers through PE.  This function uses either a blind wait
// and a very long critical section or DMA channel 8 and Timer
// 5A interrupts.
// Input: none
// Output: none
// Assumes: 80 MHz system clock, SysTick initialized, PE initialized
void WS2812_PrintBuffer(void);
