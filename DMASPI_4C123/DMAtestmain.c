// DMAtestmain.c
// Runs on LM4F120/TM4C123
// Periodic timer triggered DMA transfer
// Uses Timer5A to trigger the DMA, read from an 8-bit PORT, and then write to a memory Buffer
// There is a Timer5A interrupt after the buffer is full
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
#include "../inc/tm4c123gh6pm.h"

#include "PLL.h"
#include "DMASPI.h"
#include "MAX5353.h"
#define SSI0_DR ((volatile uint32_t *)0x40008008)

void EnableInterrupts(void);
// Blink the on-board LED.
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
uint16_t SinTable[256] = {  // must be in RAM, can't DMA out of ROM
  2048,2097,2146,2195,2244,2293,2341,2390,2438,2486,2534,2581,2629,2675,2722,2768,
  2813,2858,2903,2947,2991,3034,3076,3118,3159,3200,3239,3278,3317,3354,3391,3427,
  3462,3496,3530,3562,3594,3625,3654,3683,3711,3738,3763,3788,3812,3834,3856,3876,
  3896,3914,3931,3947,3962,3976,3988,3999,4010,4019,4026,4033,4038,4043,4046,4047,
  4048,4047,4046,4043,4038,4033,4026,4019,4010,3999,3988,3976,3962,3947,3931,3914,
  3896,3876,3856,3834,3812,3788,3763,3738,3711,3683,3654,3625,3594,3562,3530,3496,
  3462,3427,3391,3354,3317,3278,3239,3200,3159,3118,3076,3034,2991,2947,2903,2858,
  2813,2768,2722,2675,2629,2581,2534,2486,2438,2390,2341,2293,2244,2195,2146,2097,
  2048,1999,1950,1901,1852,1803,1755,1706,1658,1610,1562,1515,1467,1421,1374,1328,
  1283,1238,1193,1149,1105,1062,1020,978,937,896,857,818,779,742,705,669,634,600,
  566,534,502,471,442,413,385,358,333,308,284,262,240,220,200,182,165,149,134,120,
  108,97,86,77,70,63,58,53,50,49,48,49,50,53,58,63,70,77,86,97,108,120,134,149,165,
  182,200,220,240,262,284,308,333,358,385,413,442,471,502,534,566,600,634,669,705,
  742,779,818,857,896,937,978,1020,1062,1105,1149,1193,1238,1283,1328,1374,1421,
  1467,1515,1562,1610,1658,1706,1755,1803,1852,1901,1950,1999};


int main(void){   uint32_t t; // time in 2ms units
  volatile uint32_t delay;
  PLL_Init(Bus80MHz);  // now running at 80 MHz
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGC2_GPIOF; // enable Port F clock
  t = 0;                       // allow time to finish activating
  GPIO_PORTF_DIR_R = 0x0E;     // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0x0E; // disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x0E;    // enable digital I/O on PF3-1
  GPIO_PORTF_PCTL_R = 0;       // all pins are regular I/O
  GPIO_PORTF_AMSEL_R = 0;      // disable analog functionality on Port F
  LEDS = 0;                    // turn all LEDs off
  DAC_Init(0x1000);            // initialize with command: Vout = Vref
  DMA_Init(625);       // initialize DMA channel 8 for Timer5A transfer, every 7.8125us
  EnableInterrupts();  // Timer5A interrupt on completion, every 2ms
  DMA_Start(SinTable, SSI0_DR, 256); //7.8125us*256= 2ms period sine wave
  while(t<15000){ // 30 seconds
    while(DMA_Status()==t){    // takes 2ms seconds to transmit one sine period
//      PF0 ^= 0x01;
    }
    LEDS = COLORWHEEL[(t>>6)&0x07]; // changes every 2ms*64, about 8Hz
    t = t+1;
  }
  DMA_Stop();
  while(1){
    LEDS = COLORWHEEL[t&0x07];
    t = t+1;
    for(delay = 0; delay < 6000000; delay++){
    }
  }
}
