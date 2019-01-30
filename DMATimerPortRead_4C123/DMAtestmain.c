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
#include "DMATimer.h"


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
#define SIZE 1024
uint8_t Buffer[SIZE];
int main(void){   uint32_t t;
  PLL_Init(Bus80MHz);  // now running at 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;   // enable Port F clock
  t = 0;                       // allow time to finish, PF7-4 inputs
                               // unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R = 0x01;      // enable commit for PF0
  GPIO_PORTF_DIR_R = 0x0E;     // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0xFF; // disable alt funct on PF7-0
  GPIO_PORTF_PUR_R |= 0x11;    // enable weak pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R |= 0xFF;    // enable digital I/O on PF7-0
  GPIO_PORTF_PCTL_R = 0;       // all pins are regular I/O
  GPIO_PORTF_AMSEL_R = 0;      // disable analog functionality on Port F
  LEDS = 0;                    // turn all LEDs off
  DMA_Init(10*MS);     // initialize DMA channel 8 for Timer5A transfer, every 10ms
  EnableInterrupts();  // Timer5A interrupt on completion
  while(1){
    while(DMA_Status()){ // takes SIZE*10ms seconds to complete
    }
    DMA_Transfer(&GPIO_PORTF_DATA_R,Buffer,SIZE);
    LEDS = COLORWHEEL[t&0x07];
    t = t + 1;
  }
}
