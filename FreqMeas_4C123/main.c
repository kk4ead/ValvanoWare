// main.c
// Runs on TM4C123
// Measures frequency on PB7/T0CCP1 input
// Timer0A in periodic mode to request interrupts at 100 Hz
// Daniel Valvano,  Jonathan Valvano
// July 11. 2015

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

// oscilloscope or LED connected to PF3-1 for profiling

#include "..//inc//tm4c123gh6pm.h"
#include <stdint.h>
#include "PLL.h"
#include "FreqMeasure.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
// debugging profile 
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PF4       (*((volatile uint32_t *)0x40025040))
#define LEDS      (*((volatile uint32_t *)0x40025038))
#define RED       0x02
#define BLUE      0x04
#define GREEN     0x08
// connect PF1 to PB7
extern uint32_t Freq;   /* Frequency with units of 100 Hz */
extern int Done;
void PortF_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x20;       // activate port F
  while((SYSCTL_PRGPIO_R&0x0020) == 0){};// ready?
  GPIO_PORTF_DIR_R &= ~0x10;       // make PF4 input
  GPIO_PORTF_DIR_R |= 0x0E;        // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0x1E;     // disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x1E;        // enable digital I/O on PF3-1
                                   // configure PF3-1 as GPIO
  GPIO_PORTF_PUR_R |=0x10;          // pullup
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF0000F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PF
  LEDS = 0;                        // turn all LEDs off
}
//test code
uint32_t Time;
void SquareWave(void){uint32_t i;
  PF1 = 0;
  for(i=0; i<Time; i++){};
  PF1 = RED;
  for(i=0; i<Time; i++){}; 
}
int main(void){ 
  Time = 1;
  PLL_Init(Bus16MHz);    // bus clock at 16 MHz
  PortF_Init();
  FreqMeasure_Init();     // initialize timer0A (100 Hz)
  EnableInterrupts();

  while(1){
    while(PF4==0){SquareWave();}; // wait for release       
    while(PF4){SquareWave();};    // wait for touch 
    Done = 0;
    while(Done==0){SquareWave();} 
    PF2 ^= BLUE;
    Time = 2*Time; // slower
  }
}
