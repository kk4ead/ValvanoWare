// EdgeInterrupt.c
// Runs on LM4F120 or TM4C123
// Request an interrupt on the falling edge of PF4 (when the user
// button is pressed) and increment a counter in the interrupt.  Note
// that button bouncing is not addressed.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Section 9.5
   
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Volume 2, Section 5.5

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

// user button connected to PF4 
// call Fall() on falling edge (touch SW1)
// call Rise() on rising edge (release SW1)

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Switch.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
#define PF3                     (*((volatile uint32_t *)0x40025020))
#define PF2                     (*((volatile uint32_t *)0x40025010))
#define PF1                     (*((volatile uint32_t *)0x40025008))
uint32_t RiseCount,FallCount;
void Rise(void){
  PF3 ^= 0x08;
  RiseCount++;
}
void Fall(void){
  PF2 ^= 0x04;
  FallCount++;
}
//debug code
int main(void){ 
  SYSCTL_RCGCGPIO_R |= 0x00000020; // activate clock for port F
  while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
  GPIO_PORTF_DIR_R |= 0x0E;     // PF3,PF2,PF1 outputs
  GPIO_PORTF_DEN_R |= 0x0E;     // enable digital I/O on PF3,PF2,PF1  
  Switch_Init(&Fall,&Rise);     // initialize GPIO Port F interrupt
  EnableInterrupts();           // interrupt after all initialization

  while(1){
    Switch_WaitPress();
    Switch_WaitRelease();
    PF1 ^= 0x02;
  }
}
