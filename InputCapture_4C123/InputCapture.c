// InputCapture.c
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
#define NVIC_EN0_INT19          0x00080000  // Interrupt 19 enable

#define TIMER_CFG_16_BIT        0x00000004  // 16-bit timer configuration,
                                            // function is controlled by bits
                                            // 1:0 of GPTMTAMR and GPTMTBMR
#define TIMER_TAMR_TACMR        0x00000004  // GPTM TimerA Capture Mode
#define TIMER_TAMR_TAMR_CAP     0x00000003  // Capture mode
#define TIMER_CTL_TAEN          0x00000001  // GPTM TimerA Enable
#define TIMER_CTL_TAEVENT_POS   0x00000000  // Positive edge
#define TIMER_IMR_CAEIM         0x00000004  // GPTM CaptureA Event Interrupt
                                            // Mask
#define TIMER_ICR_CAECINT       0x00000004  // GPTM CaptureA Event Interrupt
                                            // Clear
#define TIMER_TAILR_M           0xFFFFFFFF  // GPTM Timer A Interval Load
                                            // Register


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*PeriodicTask)(void);   // user function

//------------TimerCapture_Init------------
// Initialize Timer0A in edge time mode to request interrupts on
// the rising edge of PB0 (CCP0).  The interrupt service routine
// acknowledges the interrupt and calls a user function.
// Input: task is a pointer to a user function
// Output: none
void TimerCapture_Init(void(*task)(void)){long sr;
  sr = StartCritical();
  SYSCTL_RCGCTIMER_R |= 0x01;// activate timer0
  SYSCTL_RCGCGPIO_R |= 0x02; // activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};// ready?

  PeriodicTask = task;             // user function 
  GPIO_PORTB_DIR_R &= ~0x40;       // make PB6 in
  GPIO_PORTB_AFSEL_R |= 0x40;      // enable alt funct on PB6
  GPIO_PORTB_DEN_R |= 0x40;        // enable digital I/O on PB6
                                   // configure PB6 as T0CCP0
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xF0FFFFFF)+0x07000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;     // disable analog functionality on PB6
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = TIMER_CFG_16_BIT; // configure for 16-bit timer mode
                                   // configure for capture mode, default down-count settings
  TIMER0_TAMR_R = (TIMER_TAMR_TACMR|TIMER_TAMR_TAMR_CAP);
                                   // configure for rising edge event
  TIMER0_CTL_R &= ~(TIMER_CTL_TAEVENT_POS|0xC);
  TIMER0_TAILR_R = TIMER_TAILR_M;  // max start value
  TIMER0_IMR_R |= TIMER_IMR_CAEIM; // enable capture match interrupt
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// clear timer0A capture match flag
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 16-b, +edge timing, interrupts
                                   // Timer0A=priority 2
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // top 3 bits
  NVIC_EN0_R = NVIC_EN0_INT19;     // enable interrupt 19 in NVIC
  EndCritical(sr);
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_CAECINT;// acknowledge timer0A capture match
  (*PeriodicTask)();               // execute user task
}
