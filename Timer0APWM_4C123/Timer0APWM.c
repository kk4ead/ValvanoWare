// Timer0APWM.c
// Runs on LM4F120/TM4C123
// Use Timer0A in PWM mode to generate a square wave of a given
// period and duty cycle.  Timer0A can use PF0 or PB6.  PB6 is
// used because PF0 is connected to SW2 on the Launchpad.
// NOTE:
// By default PB6 is connected to PD0 by a zero ohm R9, and PB7
// is connected to PD1 by a zero ohm R10.  These jumpers allow
// the LM4F120 Launchpad to work with certain legacy Booster
// Packs and can safely be removed.  The following program
// assumes that R9 and R10 have been removed or it is being used
// as a module in a larger program that knows not to try to use
// both PB6 and PD0 or PB7 and PD1 at the same time.
// Daniel Valvano
// September 12, 2013

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

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define TIMER_CFG_16_BIT        0x00000004  // 16-bit timer configuration,
                                            // function is controlled by bits
                                            // 1:0 of GPTMTAMR and GPTMTBMR
#define TIMER_TAMR_TAAMS        0x00000008  // GPTM TimerA Alternate Mode
                                            // Select
#define TIMER_TAMR_TAMR_PERIOD  0x00000002  // Periodic Timer mode
#define TIMER_CTL_TAEN          0x00000001  // GPTM TimerA Enable
#define TIMER_TAILR_TAILRL_M    0x0000FFFF  // GPTM TimerA Interval Load
                                            // Register Low
#define TIMER_TBILR_TBILRL_M    0x0000FFFF  // GPTM TimerB Interval Load
                                            // Register


// period is number of clock cycles in PWM period ((1/clockfreq) units)
// high is number of clock cycles output is high ((1/clockfreq) units)
// duty cycle = high/period
// assumes that period>high>(approx 3)
void PWM_Init(uint16_t period, uint16_t high){
  volatile unsigned long delay;
  SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  SYSCTL_RCGCGPIO_R |= 0x0002;     // activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};// ready?
  GPIO_PORTB_AFSEL_R |= 0x40;      // enable alt funct on PB6
  GPIO_PORTB_DEN_R |= 0x40;        // enable digital I/O on PB6
                                   // configure PB6 as T0CCP0
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xF0FFFFFF)+0x07000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;     // disable analog functionality on PB6
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // disable timer0A during setup
  TIMER0_CFG_R = TIMER_CFG_16_BIT; // configure for 16-bit timer mode
                                   // configure for alternate (PWM) mode
  TIMER0_TAMR_R = (TIMER_TAMR_TAAMS|TIMER_TAMR_TAMR_PERIOD);
  TIMER0_TAILR_R = period-1;       // timer start value
  TIMER0_TAMATCHR_R = period-high-1; // duty cycle = high/period
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // enable timer0A 16-b, PWM
}
// high is number of clock cycles output is high ((1/clockfreq) units)
// duty cycle = high/period
// assumes that period>high>(approx 3) and Timer0A PWM initialized
void PWM_Duty(uint16_t high){
  TIMER0_TAMATCHR_R = TIMER0_TAILR_R-high; // duty cycle = high/period
}
