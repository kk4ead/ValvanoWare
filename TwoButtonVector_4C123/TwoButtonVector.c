// TwoButtonVector.c
// Runs on LM4F120/TM4C123
// Use vectored interrupts to respond to two button presses.  Note
// that button bouncing is not addressed.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 9.5
   
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Volume 2, Example 5.1, Program 5.7

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

// momentary, positive logic button connected to PC4 (trigger on rising edge)
// momentary, positive logic button connected to PE4 (trigger on rising edge)

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#define NVIC_EN0_INT2           0x00000004  // Interrupt 2 enable
#define NVIC_EN0_INT4           0x00000010  // Interrupt 4 enable


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// global variables visible in Watch window of debugger
// set when corresponding button pressed
volatile uint32_t SW1, SW2;
void VectorButtons_Init(void){
  DisableInterrupts();
  // activate port C and port E
  SYSCTL_RCGCGPIO_R |= 0x14;  // enable ports C and E
  SW1 = 0;                    // clear semaphores
  SW2 = 0;
  GPIO_PORTC_DIR_R &= ~0x10;  // make PC4 in (PC4 button) (default setting)
  GPIO_PORTC_AFSEL_R &= ~0x10;// disable alt funct on PC4 (default setting)
  GPIO_PORTC_DEN_R |= 0x10;   // enable digital I/O on PC4
  GPIO_PORTC_PCTL_R &= ~0x000F0000;// configure PC4 as GPIO (default setting)
  GPIO_PORTC_AMSEL_R &= ~0x10;// disable analog functionality on PC4 (default setting)
  GPIO_PORTC_IS_R &= ~0x10;   // PC4 is edge-sensitive (default setting)
  GPIO_PORTC_IBE_R &= ~0x10;  // PC4 is not both edges (default setting)
  GPIO_PORTC_IEV_R |= 0x10;   // PC4 rising edge event
  GPIO_PORTC_ICR_R = 0x10;    // clear flag4
  GPIO_PORTC_IM_R |= 0x10;    // enable interrupt on PC4
                              // GPIO PortC=priority 2
  NVIC_PRI0_R = (NVIC_PRI0_R&0xFF00FFFF)|0x00400000; // bits 21-23
  GPIO_PORTE_DIR_R &= ~0x10;  // make PE4 in (PE4 button) (default setting)
  GPIO_PORTE_AFSEL_R &= ~0x10;// disable alt funct on PE4 (default setting)
  GPIO_PORTE_DEN_R |= 0x10;   // enable digital I/O on PE4
                              // configure PE4 as GPIO (default setting)
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFF0FFFF)+0x00000000;
  GPIO_PORTE_AMSEL_R &= ~0x10;// disable analog functionality on PE4 (default setting)
  GPIO_PORTE_IS_R &= ~0x10;   // PE4 is edge-sensitive (default setting)
  GPIO_PORTE_IBE_R &= ~0x10;  // PE4 is not both edges (default setting)
  GPIO_PORTE_IEV_R |= 0x10;   // PE4 rising edge event
  GPIO_PORTE_ICR_R = 0x10;    // clear flag4
  GPIO_PORTE_IM_R |= 0x10;    // enable interrupt on PE4
                              // GPIO PortE=priority 2
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF00)|0x00000040; // bits 5-7
                              // enable interrupts 2 and 4 in NVIC
  NVIC_EN0_R = (NVIC_EN0_INT2+NVIC_EN0_INT4);
  EnableInterrupts();
}
void GPIOPortC_Handler(void){
  GPIO_PORTC_ICR_R = 0x10;    // acknowledge flag4
  SW1 = 1;                    // signal SW1 occurred
}
void GPIOPortE_Handler(void){
  GPIO_PORTE_ICR_R = 0x10;    // acknowledge flag4
  SW2 = 1;                    // signal SW2 occurred
}

//debug code
int main(void){
  VectorButtons_Init();       // initialize interrupts and ports
  while(1){
    WaitForInterrupt();
  }
}
