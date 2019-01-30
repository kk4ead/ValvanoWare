// PWMDualTest.c
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate two square waves with 50% duty cycle
// and 180 degrees out of phase.  These two signals plus two digital logic
// inverters are used to drive a stepper motor.  The stepper motor has 200
// steps per revolution and needs to be driven between 1 and 100 revolutions
// per minute.
// 1 rev   1 min    200 step   0.25 pulse   0.83333 pulse
// ----- * ------ * -------- * ---------- = -------------
// 1 min   60 sec    1 rev       1 step         1 sec    
// Daniel Valvano
// September 10, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
  Program 6.7, section 6.3.2

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
#include "PLL.h"
#include "PWMDual.h"

void WaitForInterrupt(void);  // low power mode

int main(void){
  PLL_Init(Bus3_200MHz);           // bus clock at 3.2 MHz
  PWM0Dual_Init(60000);            // initialize PWM1-0, 0.8333 Hz, 1 RPM
//  PWM0Dual_Period(4000);           // 12.50 Hz, 15 RPM
//  PWM0Dual_Period(2400);           // 20.83 Hz, 25 RPM
//  PWM0Dual_Period(667);            // 75.00 Hz, 90 RPM
//  PWM0Dual_Period(600);            // 83.33 Hz, 100 RPM
  while(1){
    WaitForInterrupt();
  }
}
