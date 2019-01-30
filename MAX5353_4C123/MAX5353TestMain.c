// MAX5353TestMain.c
// Runs on LM4F120/TM4C123
// Test the functions provided in MAX5353.c by outputting
// a sine wave at a particular frequency.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the book
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

// SSI0Clk (SCLK, pin 4) connected to PA2
// SSI0Fss (!CS, pin 2) connected to PA3
// SSI0Tx (DIN, pin 3) connected to PA5
// see Figure 7.19 for complete schematic

#include <stdint.h>
#include "MAX5353.h"
#include "SysTick.h"

// 12-bit 32-element sine wave
// multiply each value by 2 to shift into bits 12:1 of SSI packet
// three control bits in 15:13 are all zero for immediate DAC update
// book figure shows MAX5353 in unipolar rail-to-rail configuration
// that means when wave[n] = 0x0000 (LSB = 0), output = 0
//                 wave[n] = 0x1000 (LSB = 0), output = Vref
//                 wave[n] = 0x1FFE (LSB = 0), output = 2*Vref
const uint16_t wave[32] = {
  2048*2,2448*2,2832*2,3186*2,3496*2,3751*2,3940*2,4057*2,4095*2,4057*2,3940*2,
  3751*2,3496*2,3186*2,2832*2,2448*2,2048*2,1648*2,1264*2,910*2,600*2,345*2,
  156*2,39*2,0*2,39*2,156*2,345*2,600*2,910*2,1264*2,1648*2};

int main(void){
  uint32_t i=0;
  DAC_Init(0x1000);                  // initialize with command: Vout = Vref
  SysTick_Init();
  while(1){
    DAC_Out(wave[i&0x1F]);
    i = i + 1;
    // calculated frequencies are not exact, due to the impreciseness of delay loops
    // assumes using 16 MHz PIOSC (default setting for clock source)
    // maximum frequency with 16 MHz PIOSC: (8,000,000 bits/1 sec)*(1 sample/16 bits)*(1 wave/32 sample) = 15,625 Hz
    // maximum frequency with 20 MHz PLL: (10,000,000 bits/1 sec)*(1 sample/16 bits)*(1 wave/32 sample) = 19,531.25 Hz
//    SysTick_Wait(0);                 // ?? kHz sine wave (actually 12,000 Hz)
//    SysTick_Wait(9);                 // 55.6 kHz sine wave (actually 10,000 Hz)
//    SysTick_Wait(15);                // 33.3 kHz sine wave (actually 8,500 Hz)
//    SysTick_Wait(19);                // 26.3 kHz sine wave (actually 8,500 Hz)
//    SysTick_Wait(64);                // 7.81 kHz sine wave (actually 4,800 Hz)
//    SysTick_Wait(99);                // 5.05 kHz sine wave (actually 3,500 Hz)
    SysTick_Wait(1136);              // 440 Hz sine wave (actually 420 Hz)
//    SysTick_Wait(50000);             // 10 Hz sine wave (actually 9.9 Hz)
  }
}
