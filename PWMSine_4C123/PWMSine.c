// PWMSine.c
// Runs on TM4C123
// Use Timer0A in periodic mode to request interrupts at a particular period.
// In those interrupts, change the PWM duty cycle to produce a sine wave.
// Daniel Valvano
// January 26, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 6.8, Section 6.3.2
   Program 8.7, Example 8.4

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Program 8.4, Section 8.3

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

// oscilloscope connected to PWM0A/PB6 for PWM output
// PF2 toggles on each timer interrupt (flash at frequency of sine wave * 32 / 2)

#include <stdint.h>
#include "PLL.h"
#include "PWM.h"
#include "timer0A.h"

#define PF2                     (*((volatile uint32_t *)0x40025010))
#define GPIO_PORTF_DIR_R        (*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_DEN_R        (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile uint32_t *)0x4002552C))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R5      0x00000020  // GPIO Port F Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R5        0x00000020  // GPIO Port F Peripheral Ready

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
uint32_t StartCritical (void);// previous I bit, disable interrupts
void EndCritical(uint32_t sr);// restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

const uint16_t Wave[32] = {
  125,143,159,175,189,200,208,213,215,213,208,
  200,189,175,159,143,125,107,91,75,61,50,
  42,37,35,37,42,50,61,75,91,107
};
void OutputSineWave(void){
  static uint8_t index = 0;        // counting index of output sequence
  PWM0A_Duty(Wave[index]);         // output next value in sequence
  index = (index + 1)&0x1F;        // increment counter
  PF2 ^= 0x04;
}
int main(void){
  PLL_Init(Bus50MHz);              // 50 MHz
                                   // 1) activate clock for Port F
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
                                   // allow time to finish activating
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5)==0){};
                                   // 2) no need to unlock PF2
  GPIO_PORTF_PCTL_R &= ~0x00000F00;// 3) configure PF2 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x04;     // 4) disable analog function on PF2
  GPIO_PORTF_DIR_R |= 0x04;        // 5) make PF2 out (PF2 built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;     // 6) disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;        // 7) enable digital I/O on PF2
  PWM0A_Init(250, 125);            // initialize PWM0A, 100kHz, 50% duty
//  Timer0A_Init(&OutputSineWave, 3551);// initialize 440 Hz sine wave output
  Timer0A_Init(&OutputSineWave, 1563);// initialize 1000 Hz sine wave output
  while(1){
    WaitForInterrupt();
  }
}
