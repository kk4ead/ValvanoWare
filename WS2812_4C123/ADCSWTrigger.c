// ADCSWTrigger.c
// Runs on LM4F120/TM4C123
// Provide functions that sample ADC inputs PD1, PD2, and
// PD3 using SS2 to be triggered by software and trigger
// three conversions, wait for them to finish, and return
// the three results.
// Daniel Valvano
// August 18, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
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

#include <stdint.h>
#include "ADCSWTrigger.h"
#include "../inc/lm4f120h5qr.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);  // previous I bit, disable interrupts
void EndCritical(int32_t sr); // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

void static(*UserTask2)(uint32_t x, uint32_t y, uint32_t z); // called when ADC0 SS2 finishes
void static(*UserTask3)(uint32_t microphone);                // called when ADC0 SS3 finishes

// There are many choices to make when using the ADC, and many
// different combinations of settings will all do basically the
// same thing.  For simplicity, this function makes some choices
// for you.  When calling this function, be sure that it does
// not conflict with any other software that may be running on
// the microcontroller.  Particularly, ADC0 sample sequencer 2
// is used here because it takes up to four samples, and three
// samples are needed.  Sample sequencer 2 generates a raw
// interrupt when the third conversion is complete, but it is
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
//
// This initialization function sets up the ADC according to the
// following parameters.  Any parameters not explicitly listed
// below are not modified:
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS2 triggering event: software trigger, busy-wait sampling
// SS2 1st sample source: AIN6 (PD1)
// SS2 2nd sample source: AIN5 (PD2)
// SS2 3st sample source: AIN4 (PD3)
// SS2 interrupts: enabled after 3rd sample but not promoted to controller
// 'task' is a pointer to a user function called in SS2 interrupt
// the user function has three parameters that get the x-, y-, and
// z-values from the accelerometer passed by value
void ADC0_InitSWTriggerSeq2(void(*task)(uint32_t x, uint32_t y, uint32_t z)){
  // **** GPIO pin initialization ****
  SYSCTL_RCGCGPIO_R |= 0x08;    // activate clock for Port D
                                // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&0x08) == 0){};
  GPIO_PORTD_DIR_R &= ~0x0E;    // make PD3-1 input
  GPIO_PORTD_AFSEL_R |= 0x0E;   // enable alternate function on PD3-1
  GPIO_PORTD_DEN_R &= ~0x0E;    // disable digital I/O on PD3-1
  GPIO_PORTD_AMSEL_R |= 0x0E;   // enable analog functionality on PD3-1
  // **** ADC0 SS2 initialization ****
  SYSCTL_RCGCADC_R |= 0x01;     // activate clock for ADC0
                                // allow time for clock to stabilize
  while((SYSCTL_PRADC_R&0x01) == 0){};
  ADC0_PC_R = 0x01;             // configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;        // sequencer 0 is highest, sequencer 3 is lowest
  ADC0_ACTSS_R &= ~0x04;        // disable sample sequencer 2
  ADC0_EMUX_R = (ADC0_EMUX_R&0xFFFFF0FF)+0x000; // software trigger event
  ADC0_SSMUX2_R = 0x0456;       // set channels for SS2
  ADC0_SSCTL2_R = 0x0600;       // no D0 END0 IE0 TS0 D1 END1 IE1 TS1 DO2 TS2, yes END2 IE2
  ADC0_IM_R &= ~0x04;           // disable SS2 interrupts
  ADC0_ACTSS_R |= 0x04;         // enable sample sequencer 2
  // **** function initialization ****
  UserTask2 = task;             // user function
}

//------------ADC0_InSeq2------------
// Busy-wait Analog to digital conversion initiates SS2 and
// calls the user function provided in the initialization with
// the results of the conversion in the parameters.
// UserTask2(AIN6, AIN5, AIN4);
// Input: none
// Output: none
// Samples ADC6, ADC5, and ADC4
// 125k max sampling
// software trigger, busy-wait sampling
void ADC0_InSeq2(void){
  uint32_t x, y, z;
  ADC0_PSSI_R = 0x0004;         // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0){};// 2) wait for conversion done
  x = ADC0_SSFIFO2_R&0xFFF;     // 3A) read 12-bit result from AIN6 (PD1)
  y = ADC0_SSFIFO2_R&0xFFF;     // 3B) read 12-bit result from AIN5 (PD2)
  z = ADC0_SSFIFO2_R&0xFFF;     // 3C) read 12-bit result from AIN4 (PD3)
  UserTask2(x, y, z);           // 4)  call the user function
  ADC0_ISC_R = 0x0004;          // 5) acknowledge completion
}

// This initialization function sets up the ADC according to the
// following parameters.  Any parameters not explicitly listed
// below are not modified:
// Max sample rate: <=125,000 samples/second
// Sequencer 0 priority: 1st (highest)
// Sequencer 1 priority: 2nd
// Sequencer 2 priority: 3rd
// Sequencer 3 priority: 4th (lowest)
// SS3 triggering event: software trigger, busy-wait sampling
// SS3 1st sample source: AIN7 (PD0)
// SS3 interrupts: enabled after 1st sample but not promoted to controller
// 'task' is a pointer to a user function called after SS3 finishes
// the user function has one parameter that gets the microphone
// sample passed by value
void ADC0_InitSWTriggerSeq3(void(*task)(uint32_t microphone)){
  // **** GPIO pin initialization ****
  SYSCTL_RCGCGPIO_R |= 0x08;    // activate clock for Port D
                                // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&0x08) == 0){};
  GPIO_PORTD_DIR_R &= ~0x01;    // make PD0 input
  GPIO_PORTD_AFSEL_R |= 0x01;   // enable alternate function on PD0
  GPIO_PORTD_DEN_R &= ~0x01;    // disable digital I/O on PD0
  GPIO_PORTD_AMSEL_R |= 0x01;   // enable analog functionality on PD0
  // **** ADC0 SS3 initialization ****
  SYSCTL_RCGCADC_R |= 0x01;     // activate clock for ADC0
                                // allow time for clock to stabilize
  while((SYSCTL_PRADC_R&0x01) == 0){};
  ADC0_PC_R = 0x01;             // configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;        // sequencer 0 is highest, sequencer 3 is lowest
  ADC0_ACTSS_R &= ~0x08;        // disable sample sequencer 3
  ADC0_EMUX_R = (ADC0_EMUX_R&0xFFFF0FFF)+0x0000; // software trigger event
  ADC0_SSMUX3_R = 0x0007;       // set channel for SS3
  ADC0_SSCTL3_R = 0x0006;       // no D0 TS0, yes END0 IE0
  ADC0_IM_R &= ~0x08;           // disable SS3 interrupts
  ADC0_ACTSS_R |= 0x08;         // enable sample sequencer 3
  // **** function initialization ****
  UserTask3 = task;             // user function
}

//------------ADC0_InSeq3------------
// Busy-wait Analog to digital conversion initiates SS3 and
// calls the user function provided in the initialization with
// the results of the conversion in the parameters.
// UserTask3(AIN7);
// Input: none
// Output: none
// Samples ADC7
// 125k max sampling
// software trigger, busy-wait sampling
void ADC0_InSeq3(void){
  uint32_t microphone;
  ADC0_PSSI_R = 0x0008;         // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};// 2) wait for conversion done
                                // 3) read 12-bit result from AIN7 (PD0)
  microphone = ADC0_SSFIFO3_R&0xFFF;
  UserTask3(microphone);        // 4)  call the user function
  ADC0_ISC_R = 0x0008;          // 5) acknowledge completion
}
