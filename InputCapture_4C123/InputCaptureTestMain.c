// InputCaptureTestMain.c
// Runs on LM4F120/TM4C123
// Use Timer0A in edge time mode to request interrupts on the rising
// edge of PB0 (CCP0), and count the pulses.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 6.1, Program 6.1

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
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "InputCapture.h"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t Count;      // incremented on interrupt
void UserTask(void){
  GPIO_PORTF_DATA_R = GPIO_PORTF_DATA_R^0x04; // toggle PF2
  Count = Count + 1;
}

int main(void){
                                   // activate port F
  SYSCTL_RCGCGPIO_R |= 0x20;
  while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready?
  Count = 0;                       // allow time to finish activating
  GPIO_PORTF_DIR_R |= 0x04;        // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;     // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;        // enable digital I/O on PF2
                                   // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PF
  TimerCapture_Init(&UserTask);    // initialize timer0A in capture mode
  while(1){
    WaitForInterrupt();
  }
}
