// DMAtestmain.c
// Runs on LM4F120/TM4C123
// Uses Timer5A to trigger the DMA,
//   read from a large memory Buffer, and
//   then write to an 8-bit PORT
// There is a Timer5A interrupt after the buffer has been transferred
//   In ping-pong mode the ISR sets up the next buffer
// Jonathan Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Section 6.4.5

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

#include <stdint.h>
#include "PLL.h"
#include "DMATimer.h"
#include "../inc/tm4c123gh6pm.h"

void EnableInterrupts(void);

// Blink the on-board LED.
#define GPIO_PORTE0 (*((volatile uint32_t *)0x40024004))
#define PF0       (*((volatile uint32_t *)0x40025004))
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define LEDS      (*((volatile uint32_t *)0x40025038))
#define RED       0x02
#define BLUE      0x04
#define GREEN     0x08
#define PF4       (*((volatile uint32_t *)0x40025040))
#define PF5       (*((volatile uint32_t *)0x40025080))
#define PF6       (*((volatile uint32_t *)0x40025100))
#define PF7       (*((volatile uint32_t *)0x40025200))
const int32_t COLORWHEEL[8] = {RED, RED+GREEN, GREEN, GREEN+BLUE, BLUE, BLUE+RED, RED+GREEN+BLUE, 0};
#define NUMLEDS  50        // number of LEDs in the string
#define SIZE 3*8*4*NUMLEDS // size of color encoding buffer
#define PINGPONGS 3        // number of DMA ping-pong transfers needed to send the whole encoding buffer ((SIZE/PINGPONGS/2)<=1024 and an integer)
// Each LED has three 8-bit color values stored in the order:
// [Green]  [Red]  [Blue]
// With each bit of each byte of each color of each LED stored
// in four bytes.  These four bytes encode a 0 as:
// {0x01, 0x00, 0x00, 0x00}
// and a 1 as:
// {0x01, 0x01, 0x00, 0x00}
// Note: These values are for LED string on GPIO pin 0.
// In other words, there are NUMLEDS LEDs per string of lights,
// 3 colors (green, red, blue) per LED, 8 bits per color, and
// 4 bytes per bit of color.
uint8_t Buffer[SIZE];      // the color encoding buffer
uint32_t Cursor;           // pointer into Buffer (0 to SIZE-1)

// delay function from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void Delay(uint32_t ulCount){
  __asm (  "loop    subs    r0, #1\n"
      "    bne     loop\n");
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

//------------sendreset------------
// Send a reset command to the WS2812 LED driver through PE0.
// This function uses a blind wait, so it takes at least 50
// usec to complete.
// Input: none
// Output: none
// Assumes: 80 MHz system clock, PE0 initialized
void sendreset(void){
  GPIO_PORTE0 = 0x00;              // hold data low
  Delay(1333);                     // delay ~4,000 cycles (50 usec)
//  Delay(1333);                   // delay ~4,000 cycles (50 usec)
}

//------------clearbuffer------------
// Clear the entire RAM buffer and restart the cursor to
// the beginning of the row of LEDs.  To actually update the
// physical LEDs, initiate a DMA transfer from 'Buffer' to
// GPIO_PORTE0 (0x40024004).
// Input: none
// Output: none
void clearbuffer(void){
  int i;
  Cursor = 0;                      // restart at the beginning of the buffer
  for(i=0; i<SIZE; i=i+4){
    Buffer[i] = 1;                 // clear all color values for all LEDs by encoding 0's
    Buffer[i+1] = 0;
    Buffer[i+2] = 0;
    Buffer[i+3] = 0;
  }
}

//------------addcolor------------
// Private helper function that adds a color to the buffer.
// Configure the next LED with the desired color values and
// move to the next LED automatically.  This updates the RAM
// buffer and increments the cursor.  To actually update the
// physical LEDs, initiate a DMA transfer from 'Buffer' to
// GPIO_PORTE0 (0x40024004).  A color value of zero means
// that color is not illuminated.
// Input: red   8-bit red color value
//        green 8-bit green color value
//        blue  8-bit blue color value
// Output: none
void addcolor(uint8_t red, uint8_t green, uint8_t blue){
  int i;
  for(i=0x80; i>0x00; i=i>>1){     // store the green byte first
    if(green&i){
      Buffer[Cursor] = 1;
      Buffer[Cursor+1] = 1;
      Buffer[Cursor+2] = 0;
      Buffer[Cursor+3] = 0;
    } else{
      Buffer[Cursor] = 1;
      Buffer[Cursor+1] = 0;
      Buffer[Cursor+2] = 0;
      Buffer[Cursor+3] = 0;
    }
    Cursor = Cursor + 4;
  }
  for(i=0x80; i>0x00; i=i>>1){     // store the red byte second
    if(red&i){
      Buffer[Cursor] = 1;
      Buffer[Cursor+1] = 1;
      Buffer[Cursor+2] = 0;
      Buffer[Cursor+3] = 0;
    } else{
      Buffer[Cursor] = 1;
      Buffer[Cursor+1] = 0;
      Buffer[Cursor+2] = 0;
      Buffer[Cursor+3] = 0;
    }
    Cursor = Cursor + 4;
  }
  for(i=0x80; i>0x00; i=i>>1){     // store the blue byte third
    if(blue&i){
      Buffer[Cursor] = 1;
      Buffer[Cursor+1] = 1;
      Buffer[Cursor+2] = 0;
      Buffer[Cursor+3] = 0;
    } else{
      Buffer[Cursor] = 1;
      Buffer[Cursor+1] = 0;
      Buffer[Cursor+2] = 0;
      Buffer[Cursor+3] = 0;
    }
    Cursor = Cursor + 4;
  }
  if(Cursor >= SIZE){              // check if at the end of the buffer
    Cursor = 0;                    // wrap back to the beginning
  }
}

//------------setcursor------------
// Point the cursor at the desired LED index.  This LED will
// be the next one configured by the next call to addcolor().
// If the new cursor parameter is greater than the total
// number of LEDs, it will wrap around.  For example,
// setcursor(11) with a row of 10 LEDs will wrap around to the
// second (index=1) LED.
// Input: newCursor 8-bit new cursor index
// Output: none
void setcursor(uint8_t newCursor){
  newCursor = newCursor%(SIZE/3/8/4);
  Cursor = newCursor*3*8*4;
}

int main(void){
  uint8_t phase;                   // phase value of the rainbow LED [0 to 8] or [0 to 44]
  uint8_t red, green, blue;        // value of each color
  int8_t ired, igreen, iblue;      // amount to increment each color
  int i, t;
  PLL_Init(Bus80MHz);              // now running at 80 MHz
                                   // activate clock for Port E
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
                                   // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R4) == 0){};
  GPIO_PORTE_DIR_R |= 0x01;        // make PE0 out
  GPIO_PORTE_AFSEL_R &= ~0x01;     // disable alt funct on PE0
  GPIO_PORTE_DR8R_R |= 0x01;       // enable 8 mA drive on PE0
  GPIO_PORTE_DEN_R |= 0x01;        // enable digital I/O on PE0
                                   // configure PE0 as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFFFFF0)+0x00000000;
  GPIO_PORTE_AMSEL_R &= ~0x01;     // disable analog functionality on PE0
  sendreset();
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5; // enable Port F clock
                                   // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};
                                   // unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R = 0x01;          // enable commit for PF0
  GPIO_PORTF_DIR_R = 0x0E;         // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0xFF;     // disable alt funct on PF7-0
  GPIO_PORTF_PUR_R |= 0x11;        // enable weak pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R |= 0xFF;        // enable digital I/O on PF7-0
  GPIO_PORTF_PCTL_R = 0;           // all pins are regular I/O
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on Port F
  LEDS = 0;                        // turn all LEDs off
  DMA_Init(32, (uint32_t *)&GPIO_PORTE0); // initialize DMA channel 8 for Timer5A transfer, every 24 cycles (0.4 usec)
  EnableInterrupts();              // Timer5A interrupt on completion
  phase = 0;
  setcursor(0);
  while(1){
/* Beginning of 10 LED test */
/*    if(phase < 3){
      red = 255 - 85*phase;
      green = 85*phase;
      blue = 0;
      ired = -85;
      igreen = 85;
      iblue = 0;
    } else if(phase < 6){
      red = 0;
      green = 255 - 85*(phase - 3);
      blue = 85*(phase - 3);
      ired = 0;
      igreen = -85;
      iblue = 85;
    } else{
      red = 85*(phase - 6);
      green = 0;
      blue = 255 - 85*(phase - 6);
      ired = 85;
      igreen = 0;
      iblue = -85;
    }
    phase = phase + 1;
    if(phase > 8){
      phase = 0;
    }
    for(i=0; i<9; i=i+1){
      addcolor(red, green, blue);
      red = red + ired;
      green = green + igreen;
      blue = blue + iblue;
      if((red == 255) && (ired > 0)){
        igreen = ired;
        ired = -1*ired;
      }
      if((red == 0) && (ired < 0)){
        ired = 0;
      }
      if((green == 255) && (igreen > 0)){
        iblue = igreen;
        igreen = -1*igreen;
      }
      if((green == 0) && (igreen < 0)){
        igreen = 0;
      }
      if((blue == 255) && (iblue > 0)){
        ired = iblue;
        iblue = -1*iblue;
      }
      if((blue == 0) && (iblue < 0)){
        iblue = 0;
      }
    }
    addcolor(31*phase, 31*phase, 31*phase);*/
/* End of of 10 LED test */
/* Beginning of 50 LED test */
    addcolor(5*phase, 5*phase, 5*phase);
    addcolor(0, 0, 0);
    addcolor(0, 0, 0);
    addcolor(0, 0, 0);
    addcolor(0, 0, 0);
    if(phase < 15){
      red = 240 - 16*phase;
      green = 16*phase;
      blue = 0;
      ired = -16;
      igreen = 16;
      iblue = 0;
    } else if(phase < 30){
      red = 0;
      green = 240 - 16*(phase - 15);
      blue = 16*(phase - 15);
      ired = 0;
      igreen = -16;
      iblue = 16;
    } else{
      red = 16*(phase - 30);
      green = 0;
      blue = 240 - 16*(phase - 30);
      ired = 16;
      igreen = 0;
      iblue = -16;
    }
    phase = phase + 1;
    if(phase > 44){
      phase = 0;
    }
    for(i=0; i<45; i=i+1){
      addcolor(red, green, blue);
      red = red + ired;
      green = green + igreen;
      blue = blue + iblue;
      if((red == 240) && (ired > 0)){
        igreen = ired;
        ired = -1*ired;
      }
      if((red == 0) && (ired < 0)){
        ired = 0;
      }
      if((green == 240) && (igreen > 0)){
        iblue = igreen;
        igreen = -1*igreen;
      }
      if((green == 0) && (igreen < 0)){
        igreen = 0;
      }
      if((blue == 240) && (iblue > 0)){
        ired = iblue;
        iblue = -1*iblue;
      }
      if((blue == 0) && (iblue < 0)){
        iblue = 0;
      }
    }
/* End of 50 LED test */
    sendreset();         // send a reset
    DMA_Transfer((uint8_t *)Buffer, SIZE/PINGPONGS, PINGPONGS);
    LEDS = COLORWHEEL[t&0x07];
    t = t + 1;
    Delay(2000000);
    while(DMA_Status() != IDLE){// takes SIZE*0.4 microseconds to complete
    }
  }
}
