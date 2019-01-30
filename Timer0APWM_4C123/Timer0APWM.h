// Timer0APWM.h
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

// period is number of clock cycles in PWM period ((1/clockfreq) units)
// high is number of clock cycles output is high ((1/clockfreq) units)
// duty cycle = high/period
// assumes that period>high>(approx 3)
void PWM_Init(uint16_t period, uint16_t high);
// high is number of clock cycles output is high ((1/clockfreq) units)
// duty cycle = high/period
// assumes that period>high>(approx 3) and Timer0A PWM initialized
void PWM_Duty(uint16_t high);
