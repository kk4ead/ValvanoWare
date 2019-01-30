// MAX549TestMain.c
// Runs on LM4F120/TM4C123
// Test the functions provided in MAX549.c by outputting
// a sine wave at a particular frequency.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

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
// Max549 pin 1 ground
// Max549 pin 2 OUTA
// Max549 pin 3 CS, SSI0Fss connected to PA3
// Max549 pin 4 DIN, SSI0Tx connected to PA5
// Max549 pin 5 SCLK SSI0Clk connected to PA2
// Max549 pin 6 OUTB
// Max549 pin 7 REF (cheap solution connects this to +3.3V)
// Max549 pin 8 +3.3V
#include <stdint.h>
#include "MAX549.h"
#include "SysTick.h"

// 8-bit 64-element sine wave
const uint16_t wave[64] = {
  128,140,153,165,177,188,199,209,218,226,234,
  240,245,250,253,254,255,254,253,250,245,240,
  234,226,218,209,199,188,177,165,153,140,
  128,116,103,91,79,68,57,47,38,30,
  22,16,11,6,3,2,1,2,3,6,11,
  16,22,30,38,47,57,68,79,91,103,116};

int main(void){
  uint32_t i=0,j=32;
  DAC_Init(128);                  // initialize 
  SysTick_Init();
  while(1){
    DAC_Out(wave[i&0x3F],wave[j&0x3F]);
    i = i+1;
    j = j+1;
    SysTick_Wait(1136);              // 440 Hz sine wave (actually 420 Hz)
  }
}
