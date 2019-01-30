// WS2812.c
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

#include <stdint.h>
#include "DMATimer.h"
#include "PLL.h"
#include "SysTick.h"
#include "WS2812.h"

#define GPIO_PORTE_BASE         (*((volatile uint32_t *)0x40024000))
#define GPIO_PORTE_DIR_R        (*((volatile uint32_t *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile uint32_t *)0x40024420))
#define GPIO_PORTE_DR8R_R       (*((volatile uint32_t *)0x40024508))
#define GPIO_PORTE_DEN_R        (*((volatile uint32_t *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile uint32_t *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile uint32_t *)0x4002452C))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R4      0x00000010  // GPIO Port E Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R4        0x00000010  // GPIO Port E Peripheral Ready
#define NVIC_ST_CURRENT_R       (*((volatile uint32_t *)0xE000E018))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);  // previous I bit, disable interrupts
void EndCritical(int32_t sr); // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// delay function from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void Delay(uint32_t ulCount){
  __asm (  "    subs    r0, #1\n"
      "    bne     Delay\n"
      "    bx      lr\n");
}

#else
  //Keil uVision Code
  __asm void
  Delay(uint32_t ulCount)
  {
    subs    r0, #1
    bne     Delay
    bx      lr
  }

#endif

// Global variables to build the buffer in RAM.  Once the LEDs
// are configured as desired, all bits are sent at once.
// The cursors are managed automatically.

// Each LED has three 8-bit color values stored in the order:
// [Green]  [Red]  [Blue]
// With each bit of each byte of each color of each LED stored
// in four bytes.  These four bytes encode a 0 as:
// {0x01, 0x00, 0x00, 0x00}
// and a 1 as:
// {0x01, 0x01, 0x00, 0x00}
// In other words, there are NUMCOLS LEDs per string of lights,
// 3 colors (green, red, blue) per LED, 8 bits per color, and
// 4 bytes per bit of color.
uint8_t buffer[3*8*4*NUMCOLS];
uint32_t cursor[NUMROWS];

//------------WS2812_Init------------
// Initialize all peripherals needed to interface with a WS2812.
// Set system clock to 80 MHz using the PLL and initialize
// either the SysTick timer without interrupts or DMA channel 8
// with Timer5A.  Choose the transfer mode at compile time using
// "USEDMA" in WS2812.h.  Configure PE as GPIO out.
// Input: none
// Output: none
void WS2812_Init(void){
  PLL_Init();                      // set clock to 80 MHz
                                   // activate clock for Port E
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
                                   // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R4) == 0){};
  GPIO_PORTE_DIR_R |= ROWMASK;     // make PE out
  GPIO_PORTE_AFSEL_R &= ~ROWMASK;  // disable alt funct on PE
  GPIO_PORTE_DR8R_R |= ROWMASK;    // enable 8 mA drive on PE
  GPIO_PORTE_DEN_R |= ROWMASK;     // enable digital I/O on PE
                                   // configure PE as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&ROWMASK2)+(0*ROWMASK);
  GPIO_PORTE_AMSEL_R &= ~ROWMASK;  // disable analog functionality on PE
#if USEDMA
  DMA_Init(32, (uint32_t *)(0x40024000 + (ROWMASK<<2))); // initialize DMA channel 8 for Timer5A transfer, every 24 cycles (0.4 usec)
#else
  SysTick_Init();                  // initialize SysTick timer
#endif
  WS2812_ClearBuffer();            // clear the buffer
  WS2812_SendReset();              // send the reset pulse
}

//------------WS2812_SendReset------------
// Send a reset command to all WS2812 LED drivers connected
// to PE.  This function uses a blind wait, so it takes at
// least 50 usec to complete.
// Input: none
// Output: none
// Assumes: 80 MHz system clock, PE initialized
void WS2812_SendReset(void){
#if USEDMA
  while(DMA_Status() == BUSY){};   // wait for DMA transfer to complete
#endif
                                   // hold data low
  *(uint32_t *)(0x40024000 + (ROWMASK<<2)) = 0x00;
  Delay(1333);                     // delay ~4,000 cycles (50 usec)
}

//------------WS2812_ClearBuffer------------
// Clear the entire RAM buffer and restart all cursors to
// the beginning of the row of LEDs.  To actually update the
// physical LEDs, call WS2812_PrintBuffer().
// Input: none
// Output: none
void WS2812_ClearBuffer(void){
  uint32_t i;
#if USEDMA
  while(DMA_Status() == BUSY){};   // wait for DMA transfer to complete
#endif
  for(i=0; i<NUMROWS; i=i+1){
    cursor[i] = 0;                 // restart at the beginning of the buffer
  }
  for(i=0; i<(3*8*4*NUMCOLS); i=i+4){
    buffer[i] = ROWMASK;           // clear all color values for all LEDs by encoding 0's
    buffer[i+1] = 0;
    buffer[i+2] = 0;
    buffer[i+3] = 0;
  }
}

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
void WS2812_AddColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t row){
  int i, mask;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
#if USEDMA
  while(DMA_Status() == BUSY){};   // wait for DMA transfer to complete
#endif
  mask = 1<<row;                   // bit mask of LED row to modify
  for(i=0x80; i>0x00; i=i>>1){     // store the green byte first
    if(green&i){
//      buffer[cursor[row]] |= mask;
      buffer[cursor[row]+1] |= mask;
//      buffer[cursor[row]+2] &= ~mask;
//      buffer[cursor[row]+3] &= ~mask;
    } else{
//      buffer[cursor[row]] |= mask;
      buffer[cursor[row]+1] &= ~mask;
//      buffer[cursor[row]+2] &= ~mask;
//      buffer[cursor[row]+3] &= ~mask;
    }
    cursor[row] = cursor[row] + 4;
  }
  for(i=0x80; i>0x00; i=i>>1){     // store the red byte second
    if(red&i){
//      buffer[cursor[row]] |= mask;
      buffer[cursor[row]+1] |= mask;
//      buffer[cursor[row]+2] &= ~mask;
//      buffer[cursor[row]+3] &= ~mask;
    } else{
//      buffer[cursor[row]] |= mask;
      buffer[cursor[row]+1] &= ~mask;
//      buffer[cursor[row]+2] &= ~mask;
//      buffer[cursor[row]+3] &= ~mask;
    }
    cursor[row] = cursor[row] + 4;
  }
  for(i=0x80; i>0x00; i=i>>1){     // store the blue byte third
    if(blue&i){
//      buffer[cursor[row]] |= mask;
      buffer[cursor[row]+1] |= mask;
//      buffer[cursor[row]+2] &= ~mask;
//      buffer[cursor[row]+3] &= ~mask;
    } else{
//      buffer[cursor[row]] |= mask;
      buffer[cursor[row]+1] &= ~mask;
//      buffer[cursor[row]+2] &= ~mask;
//      buffer[cursor[row]+3] &= ~mask;
    }
    cursor[row] = cursor[row] + 4;
  }
  if(cursor[row] >= (3*8*4*NUMCOLS)){
                                   // cursor has reached the end of the buffer
    cursor[row] = 0;               // wrap back to the beginning
  }
}

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
void WS2812_SetCursor(uint8_t newCursor, uint8_t row){
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  newCursor = newCursor%NUMCOLS;
  cursor[row] = newCursor*3*8*4;
}

//------------WS2812_GetCursor------------
// Return the current cursor index.  This is the index of the
// LED that will be the next one configured by the next call
// to WS2812_AddColor().
// Input: row  index of LED row (0<=row<NUMROWS)
// Output: 8-bit current cursor index
uint8_t WS2812_GetCursor(uint8_t row){
  if(row >= NUMROWS){
    return 0;                      // row parameter is invalid, this value is meaningless
  }
  return cursor[row]/3/8/4;
}

//------------WS2812_PrintBuffer------------
// Send all 24-bit color values to the chain of WS2812 LED
// drivers through PE.  This function uses either a blind wait
// and a very long critical section or DMA channel 8 and Timer
// 5A interrupts.
// Input: none
// Output: none
// Assumes: 80 MHz system clock, SysTick initialized, PE initialized
#if USEDMA
void WS2812_PrintBuffer(void){
  if(DMA_Status() != IDLE){        // handle error case where DMA channel is busy
                                   // you can wait for it to finish in the background, but this
                                   // will crash if not initialized, and even if it recovers,
    return;                        // the data will immediately follow and thus not be displayed
  }
  DMA_Transfer((uint8_t *)buffer, 1728, 2); // send 18 LEDs with each ping-pong; send 2 ping-pongs
}
#else
/*void WS2812_PrintBuffer(void){
  uint32_t i;
  uint32_t startTime;              // starting time on SysTick
  uint32_t *portE;                 // pointer to GPIO Port E
  portE = (uint32_t *)(GPIO_PORTE_BASE + (ROWMASK<<2));
  DisableInterrupts();             // begin critical section
  startTime = NVIC_ST_CURRENT_R;
  *portE = buffer[0];              // send the first byte
                                   // send each byte of the buffer in order
  for(i=1; i<(3*8*4*NUMCOLS); i=i+1){
                                   // wait any remaining time from previous delay
    while(((startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF) < 24){};
    startTime = NVIC_ST_CURRENT_R;
    *portE = buffer[i];
  }
                                   // wait any remaining time from previous delay
  while(((startTime-NVIC_ST_CURRENT_R)&0x00FFFFFF) < 24){};
  EnableInterrupts();              // end of critical section
}*/

#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void WS2812_PrintBuffer(void){
  __asm (  "    IMPORT buffer\n"
      "    CPSID I\n"
      "    PUSH {R4, R5, R6, R7, R8, R9, R10, R11}\n"
      "    LDR R4, =0xE000E010\n"
      "    LDR R5, =0x00010005\n"
      "    LDR R6, =0xE000E014\n"
      "    MOV R7, #16\n"
      "    LDR R8, =0xE000E018\n"
      "    LDR R9, =buffer\n"
      "    LDR R10, =0x40024000\n"
      "    MOV R0, #ROWMASK\n"
      "    LSL R0, R0, #2\n"
      "    ADD R10, R10, R0\n"
      "    MOV R11, #0\n"
      "WS2812_PrintBufferMain\n"
      "    LDRB R0, [R9], #1\n"
      "    ADD R11, R11, #1\n"
      "    STR R7, [R6]\n"
      "    STR R7, [R8]\n"
      "    STR R0, [R10]\n"
      "WS2812_PrintBufferWait\n"
      "    LDR R0, [R4]\n"
      "    CMP R0, R5\n"
      "    BNE WS2812_PrintBufferWait\n"
      "    CMP R11, #(3*8*4*NUMCOLS)\n"
      "    BLO WS2812_PrintBufferMain\n"
      "    POP {R4, R5, R6, R7, R8, R9, R10, R11}\n"
      "    CPSIE I\n"
      "    BX  LR\n");
}

#else
  //Keil uVision Code
  __asm void
  WS2812_PrintBuffer(void)
  {
    IMPORT buffer
    CPSID I
    PUSH {R4, R5, R6, R7, R8, R9, R10, R11}  // save current value of R4-R11
    LDR R4, =0xE000E010            // R4 = &NVIC_ST_CTRL_R (pointer)
    LDR R5, =0x00010005            // R5 = SysTick control value
    LDR R6, =0xE000E014            // R6 = &NVIC_ST_RELOAD_R (pointer)
    MOV R7, #16                    // R7 = SysTick reload value
    LDR R8, =0xE000E018            // R8 = &NVIC_ST_CURRENT_R (pointer)
    LDR R9, =buffer                // R9 = &buffer (pointer)
    LDR R10, =0x40024000           // R10 = &GPIO_PORTE_BASE (pointer)
    MOV R0, #ROWMASK               // R0 = ROWMASK (value)
    LSL R0, R0, #2                 // R0 = R0<<2
    ADD R10, R10, R0               // R10 = R10 + R0 (&GPIO_PORTE_BASE + ROWMASK<<2) (pointer)
    MOV R11, #0                    // R11 = 0 (number of runs through loop)
WS2812_PrintBufferMain
    LDRB R0, [R9], #1              // R0 = buffer (8-bit value); R9 = R9 + 1 (increment buffer pointer to next byte)
    ADD R11, R11, #1               // R11 = R11 + 1 (increment counts through loop)
    STR R7, [R6]                   // [R6] = R7 (NVIC_ST_RELOAD_R = 16)
    STR R7, [R8]                   // [R8] = R7 (NVIC_ST_CURRENT_R = 16; any write clears it)
    STR R0, [R10]                  // [R10] = R0 (GPIO_PORTE = buffer[R11])
WS2812_PrintBufferWait
    LDR R0, [R4]                   // R0 = NVIC_ST_CTRL_R (value)
    CMP R0, R5                     // is R0 (NVIC_ST_CTRL_R) == R5 (0x00010005) ?
    BNE WS2812_PrintBufferWait     // if not, branch to 'WS2812_PrintBufferWait'
    CMP R11, #(3*8*4*NUMCOLS)      // is R11 (runs through loop) < (3*8*4*NUMCOLS) (maximum runs through loop) ?
    BLO WS2812_PrintBufferMain     // if so, branch to 'WS2812_PrintBufferMain'
    POP {R4, R5, R6, R7, R8, R9, R10, R11}   // restore previous value of R4-R11
    CPSIE I
    BX  LR                         // return
  }

#endif
#endif
