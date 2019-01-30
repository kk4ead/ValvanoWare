// Timer0APWMTestMain.c
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
#include "Timer0APWM.h"
#include "PLL.h"

void WaitForInterrupt(void);  // low power mode

int main(void){
  PLL_Init(Bus80MHz);               // 80 MHz clock
//  PWM_Init(period, high);
//  pulse frequency = 16,000,000/period (Hz)
//  pulse duty cycle = high/period
//  PWM_Init(320, 256);             // initialize timer0A in PWM mode (50,000 Hz, 80% duty cycle)
//  PWM_Init(800, 760) ;            // initialize timer0A in PWM mode (20,000 Hz, 95% duty cycle)
//  PWM_Init(1067, 160);            // initialize timer0A in PWM mode (15,000 Hz, 15% duty cycle)
//  PWM_Init(16000, 12000);         // initialize timer0A in PWM mode (1,000 Hz, 75% duty cycle)
//  PWM_Init(16000, 8000);          // initialize timer0A in PWM mode (1,000 Hz, 50% duty cycle)
  PWM_Init(16000, 4000);          // initialize timer0A in PWM mode (1,000 Hz, 25% duty cycle)
//  PWM_Init(2, 1);          // initialize timer0A in PWM mode (8MHz, 50% duty cycle)
  while(1){
    WaitForInterrupt();
  }
}
