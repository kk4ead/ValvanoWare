// ADCSWTrigger.h
// Runs on LM4F120/TM4C123
// Provide functions that sample ADC inputs PE4, PE5 using SS2
// to be triggered by software and trigger two conversions,
// wait for them to finish, and return the two results.
// Daniel Valvano
// May 2, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
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


// There are many choices to make when using the ADC, and many
// different combinations of settings will all do basically the
// same thing.  For simplicity, this function makes some choices
// for you.  When calling this function, be sure that it does
// not conflict with any other software that may be running on
// the microcontroller.  Particularly, ADC0 sample sequencer 2
// is used here because it takes up to four samples, and two
// samples are needed.  Sample sequencer 2 generates a raw
// interrupt when the second conversion is complete, but it is
// not promoted to a controller interrupt.  Software triggers
// the ADC0 conversion and waits for the conversion to finish.
// If somewhat precise periodic measurements are required, the
// software trigger can occur in a periodic interrupt.  This
// approach has the advantage of being simple.  However, it does
// not guarantee real-time.
//
// A better approach would be to use a hardware timer to trigger
// the ADC conversion independently from software and generate
// an interrupt when the conversion is finished.  Then, the
// software can transfer the conversion result to memory and
// process it after all measurements are complete.

// Initializes ADC8 and ADC9 sampling
// 125k max sampling
// SS2 triggering event: software trigger, busy-wait sampling
// SS2 1st sample source: Ain9 (PE4)
// SS2 2nd sample source: Ain8 (PE5)
// SS2 interrupts: enabled after 2nd sample but not promoted to controller
void ADC_Init89(void);

//------------ADC_In89------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: two 12-bit result of ADC conversions
// Samples ADC8 and ADC9 
// 125k max sampling
// software trigger, busy-wait sampling
// data returned by reference
// data[0] is ADC8 (PE5) 0 to 4095
// data[1] is ADC9 (PE4) 0 to 4095
void ADC_In89(uint32_t data[2]);
