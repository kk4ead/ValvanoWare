// FreqMeasure.h
// Runs on TM4C123
// Measures frequency on PB7/T0CCP1 input
// Timer0A in periodic mode to request interrupts at 100 Hz
// Daniel Valvano,  Jonathan Valvano
// July 11, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
  Program 6.9, example 6.8

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

#ifndef __FREQMEASURE_H__ // do not include more than once
#define __FREQMEASURE_H__

// ***************** FreqMeasure_Init ****************
// Activate Timer0A interrupts to run user task periodically
// Count rising edges on T0CCP1/PB7
// Assumes 16 MHz bus
// Inputs:  none
// Outputs: none
void FreqMeasure_Init(void);

#endif // __FREQMEASURE_H__
