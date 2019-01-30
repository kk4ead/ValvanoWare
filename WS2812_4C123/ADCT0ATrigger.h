// ADCT0ATrigger.h
// Runs on LM4F120/TM4C123
// Provide a function that initializes Timer0A to trigger ADC
// conversions and request an interrupt when the conversion is
// complete.
// Daniel Valvano
// August 18, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// This initialization function sets up the ADC according to the
// following parameters.  Any parameters not explicitly listed
// below are not modified:
// Timer0: enabled
// Mode: 32-bit, down counting (NOTE: also uses Timer0B)
// One-shot or periodic: periodic
// Interval value: programmable using variable 'period' [0:4294967296]
// Sample time is busPeriod*period
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS3 triggering event: Timer0
// SS3 1st sample source: AIN7 (PD0)
// SS3 interrupts: enabled and promoted to controller
// 'task' is a pointer to a user function called in SS3 interrupt
// the user function has one parameter that gets the microphone
// sample passed by value
void ADC0_InitTimer0ATriggerSeq3(uint32_t period, void(*task)(uint32_t microphone));
