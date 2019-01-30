// ADCT0ATrigger.c
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

#include <stdint.h>
#include "ADCT0ATrigger.h"
#include "../inc/tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);  // previous I bit, disable interrupts
void EndCritical(int32_t sr); // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

void static(*UserTask3)(uint32_t microphone);                // called in ADC0 SS3 interrupt

// There are many choices to make when using the ADC, and many
// different combinations of settings will all do basically the
// same thing.  For simplicity, this function makes some choices
// for you.  When calling this function, be sure that it does
// not conflict with any other software that may be running on
// the microcontroller.  Particularly, ADC0 sample sequencer 2
// is used here because it takes up to four samples, and three
// samples are needed.  Sample sequencer 3 generates a raw
// interrupt when the third conversion is complete, and it is
// then promoted to an ADC0 controller interrupt.  Hardware
// Timer0 triggers the ADC0 conversion at the programmed
// interval, and software handles the interrupt to process the
// measurement when it is complete.
//
// A simpler approach would be to use software to trigger the
// ADC0 conversion, wait for it to complete, and then process the
// measurement.
//
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
void ADC0_InitTimer0ATriggerSeq3(uint32_t period, void(*task)(uint32_t microphone)){
  // **** GPIO pin initialization ****
  SYSCTL_RCGCGPIO_R |= 0x08;    // activate clock for Port D
                                // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&0x08) == 0){};
  GPIO_PORTD_DIR_R &= ~0x01;    // make PD0 input
  GPIO_PORTD_AFSEL_R |= 0x01;   // enable alternate function on PD0
  GPIO_PORTD_DEN_R &= ~0x01;    // disable digital I/O on PD0
  GPIO_PORTD_AMSEL_R |= 0x01;   // enable analog functionality on PD0
  // **** Timer0A initialization ****
  SYSCTL_RCGCTIMER_R |= 0x01;   // activate clock for Timer0
                                // allow time for clock to stabilize
  while((SYSCTL_PRTIMER_R&0x01) == 0){};
  TIMER0_CTL_R &= ~0x00000001;  // disable Timer0A during setup
  TIMER0_CTL_R |= 0x00000020;   // enable Timer0A trigger to ADC
  TIMER0_CFG_R = 0;             // configure for 32-bit timer mode
  TIMER0_TAMR_R = 0x00000002;   // configure for periodic mode, default down-count settings
  TIMER0_TAPR_R = 0;            // prescale value not available in 32-bit mode
  TIMER0_TAILR_R = period - 1;  // start value for trigger
  TIMER0_IMR_R = 0x00000000;    // disable all interrupts
  TIMER0_CTL_R |= 0x00000001;   // enable Timer0A 32-b, periodic, no interrupts
  // **** ADC0 SS3 initialization ****
  SYSCTL_RCGCADC_R |= 0x01;     // activate clock for ADC0
                                // allow time for clock to stabilize
  while((SYSCTL_PRADC_R&0x01) == 0){};
  ADC0_PC_R = 0x01;             // configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;        // sequencer 0 is highest, sequencer 3 is lowest
  ADC0_ACTSS_R &= ~0x08;        // disable sample sequencer 3
  ADC0_EMUX_R = (ADC0_EMUX_R&0xFFFF0FFF)+0x5000; // timer trigger event
  ADC0_SSMUX3_R = 7;            // PD0 is channel 7
  ADC0_SSCTL3_R = 0x06;         // set flag and end
  ADC0_IM_R |= 0x08;            // enable SS3 interrupts
  ADC0_ACTSS_R |= 0x08;         // enable sample sequencer 3
  // **** NVIC interrupt initialization ****
  NVIC_PRI4_R = (NVIC_PRI4_R&0xFFFF00FF)|0x00004000; // priority 2
  NVIC_EN0_R = 1<<17;           // enable interrupt 17 in NVIC
  // **** function initialization ****
  UserTask3 = task;             // user function
}

void ADC0Seq3_Handler(void){
  uint32_t microphone;
  microphone = ADC0_SSFIFO3_R&0xFFF;// 12-bit result from AIN7 (PD0)
  UserTask3(microphone);
  ADC0_ISC_R = 0x0008;          // acknowledge ADC sequence 3 completion
}
