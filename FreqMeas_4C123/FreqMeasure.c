// FreqMeasure.c
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
#include <stdint.h>
#include "..//inc//tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

/* Set each measurement, every 10 ms */
uint32_t Freq;   /* Frequency with units of 100 Hz */
int Done;

// ***************** FreqMeasure_Init ****************
// Activate Timer0A interrupts to run user task periodically
// Count rising edges on T0CCP1/PB7
// Assumes 16 MHz bus
// Inputs:  none
// Outputs: none
void FreqMeasure_Init(void){long sr;
  sr = StartCritical(); 
  SYSCTL_RCGCTIMER_R |= 0x01;     // activate timer0
  SYSCTL_RCGCGPIO_R |= 0x02;      // activate port B
  Freq = 0;                       // allow time to finish activating
  Done = 0;
  GPIO_PORTB_DIR_R &= ~0x80;      // make PB7 inputs
  GPIO_PORTB_DEN_R |= 0x80;       // enable digital PB7
  GPIO_PORTB_AFSEL_R |= 0x80;     // enable alt funct on PB7
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x0FFFFFFF)+0x70000000;
  TIMER0_CTL_R = 0x00000000;      // disable TIMER0A TIMER0B during setup
  TIMER0_CFG_R = 0x00000004;      // configure for 16-bit mode
  TIMER0_TAMR_R = 0x00000002;     // configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = 9999;          // reload value 100 Hz
  TIMER0_TAPR_R = 15;             // 1us resolution
  TIMER0_TBMR_R = 0x00000003;     // edge count mode
  // bits 11-10 are 0 for rising edge counting on PB7
  TIMER0_TBILR_R = 0xFFFFFFFF;    // start value
  TIMER0_TBPR_R = 0xFF;           // activate prescale, creating 24-bit 
  TIMER0_IMR_R &= ~0x700;         // disable all interrupts for timer0B
  TIMER0_ICR_R = 0x00000001;      // clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;      // arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;             // enable IRQ 19 in NVIC
  TIMER0_CTL_R |= 0x00000101;     // enable TIMER0A TIMER0B
  EndCritical(sr);
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = 0x00000001;      // acknowledge timer0A timeout
  Freq = (0xFFFFFF-TIMER0_TBR_R); // f = (pulses)/(fixed time)
  Done = -1;
  TIMER0_CTL_R &= ~0x00000100;    // disable TIMER0B
  TIMER0_TBILR_R = 0xFFFFFFFF;    // start value
  TIMER0_TBPR_R = 0xFF;           // activate prescale, creating 24-bit 
  TIMER0_CTL_R |= 0x00000100;     // enable TIMER0B
}
