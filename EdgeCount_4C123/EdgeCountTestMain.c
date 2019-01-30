// EdgeCountTestMain.c
// Runs on LM4F120/TM4C123
// Use Timer0A in edge count mode to increment TIMER0A on
// rising edge of PB6 (CCP0).
// Jonathan Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 6.xx, Program 6.xx

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

// external signal connected to PB6 (T0CCP0) (trigger on rising edge)
// for testing purposes, PD0 is used as an output connected to PB6
//  remove R9 or change the following software to use PB6 (T0CCP0) independently

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "EdgeCount.h"

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void Delay(unsigned long ulCount){
  __asm (  "loop    subs    r0, #1\n"
      "    bne     loop\n");
}

#else
  //Keil uVision Code
  __asm void
  Delay(unsigned long ulCount)
  {
    subs    r0, #1
    bne     Delay
    bx      lr
  }

#endif

volatile uint32_t Count;      // incremented on edge
volatile uint16_t Errors;

// helper function delays until sw2 is pressed and released, toggling the LED
void waitforsw2touch(void){
  // wait for SW2 pressed
  while((GPIO_PORTF_DATA_R&0x01) == 0x01){
    if(Errors == 0){
      GPIO_PORTF_DATA_R |= 0x08;
      Delay(1333333);        // wait ~0.25 sec
      GPIO_PORTF_DATA_R &= ~0x08;
      Delay(1333333);        // wait ~0.25 sec
    } else{
      GPIO_PORTF_DATA_R |= 0x02;
      Delay(1333333);        // wait ~0.25 sec
      GPIO_PORTF_DATA_R &= ~0x02;
      Delay(1333333);        // wait ~0.25 sec
    }
  }
  // wait for SW2 released
  while((GPIO_PORTF_DATA_R&0x01) == 0x00){};
}

// helper function pulses PD0 with 'delay' cycles high and low
void pulse(uint32_t delay){
  GPIO_PORTD_DATA_R |= 0x01;
  Delay(delay/3);
  GPIO_PORTD_DATA_R &= ~0x01;
  Delay(delay/3);
}

int main(void){
  uint32_t i;
  // initialize built-in switches and LEDs on Port F
  SYSCTL_RCGCGPIO_R |= 0x20; // activate Port F
  while((SYSCTL_PRGPIO_R&0x20) == 0){};// allow time to finish activating
                             // unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R |= 0x01;   // enable commit for PF0
  GPIO_PORTF_DIR_R |= 0x0E;  // make PF3-1 out (built-in LEDs)
  GPIO_PORTF_DIR_R &= ~0x01; // make PF0 in (built-in SW2)
  GPIO_PORTF_AFSEL_R &= ~0x0F;// disable alt funct on PF3-0
  GPIO_PORTF_PUR_R |= 0x01;  // enable pull-up on PF0
  GPIO_PORTF_DEN_R |= 0x0F;  // enable digital I/O on PF3-0
                             // configure PF3-0 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF0000)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;    // disable analog functionality on PF
  GPIO_PORTF_DATA_R = 0;
  // initialize debugging output on Port D0
  SYSCTL_RCGCGPIO_R |= 0x08; // activate Port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// allow time to finish activating
  GPIO_PORTD_DIR_R |= 0x01;  // make PD0 out
  GPIO_PORTD_AFSEL_R &= ~0x01;// disable alt funct on PD0
  GPIO_PORTD_DEN_R |= 0x01;  // enable digital I/O on PD0
                             // configure PD0 as GPIO
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFFFFF0)+0x00000000;
  GPIO_PORTD_AMSEL_R &= ~0x01;// disable analog functionality on PD0
  GPIO_PORTD_DATA_R = 0;
  // initialize Timer0A
  EdgeCount_Init();          // initialize Timer0A in Edge Count mode
  // initialize variables
  Count = 0;
  Errors = 0;
  // first test: initial count should be 0
  Count = EdgeCount_Result();
  if(Count != 0){
    Errors = Errors + 1;
  }
  waitforsw2touch();         // press and release SW2 to continue
  // second test: count 5 pulses
  for(i=0; i<5; i=i+1){
    pulse(480);              // pulse PD0 over ~960 cycles (~60 usec)
  }
  Count = EdgeCount_Result();
  if(Count != 5){
    Errors = Errors + 1;
  }
  waitforsw2touch();         // press and release SW2 to continue
  // third test: count 10 more pulses
  for(i=0; i<10; i=i+1){
    pulse(480);              // pulse PD0 over ~960 cycles (~60 usec)
  }
  Count = EdgeCount_Result();
  if(Count != 15){
    Errors = Errors + 1;
  }
  waitforsw2touch();         // press and release SW2 to continue
  // fourth test: count 16,777,200 more pulses
  for(i=0; i<16777200; i=i+1){
    pulse(9);                // pulse PD0 over ~18 cycles (~0.5625 usec)
  }
  Count = EdgeCount_Result();
  if(Count != 16777215){     // 0xFFFFFF
    Errors = Errors + 1;
  }
  waitforsw2touch();         // press and release SW2 to continue
  // fifth test: count 10 more pulses
  // the first one should wrap around to 0, then 9 more
  for(i=0; i<10; i=i+1){
    pulse(480);              // pulse PD0 over ~960 cycles (~60 usec)
  }
  Count = EdgeCount_Result();
  if(Count != 9){
    Errors = Errors + 1;
  }
  waitforsw2touch();         // press and release SW2 to continue
  // sixth test: count 16,777,216 more pulses
  // this should wrap all the way around, causing EdgeCount_Result() to appear unchanged
  for(i=0; i<16777216; i=i+1){
    pulse(9);                // pulse PD0 over ~18 cycles (~0.5625 usec)
  }
  Count = EdgeCount_Result();
  if(Count != 9){
    Errors = Errors + 1;
    GPIO_PORTF_DATA_R |= 0x02;
  }
  while(1){
    Count = EdgeCount_Result();
    GPIO_PORTF_DATA_R |= 0x04;
    Delay(5333333);          // wait ~0.1 sec
    GPIO_PORTF_DATA_R &= ~0x04;
    Delay(5333333);          // wait ~0.1 sec
  }
}
