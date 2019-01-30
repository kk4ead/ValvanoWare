// PWMDual.h
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate two square waves with 50% duty cycle
// and 180 degrees out of phase.
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


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/64 
//                = 3.2 MHz/64 = 50 kHz (in this example)
void PWM0Dual_Init(uint16_t period);

// change period
// period is 16-bit number of PWM clock cycles in one period (3<=period)
void PWM0Dual_Period(uint16_t period);
