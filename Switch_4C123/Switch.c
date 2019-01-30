// Switch.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize a GPIO as an input pin and
// allow reading of two negative logic switches on PF0 and PF4
// and an external switch on PA5.
// Use bit-banded I/O.
// Daniel and Jonathan Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Section 4.2    Program 4.2

  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 2.3, Program 2.9, Figure 2.36

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

// negative logic switches connected to PF0 and PF4 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad
// NOTE: The NMI (non-maskable interrupt) is on PF0.  That means that
// the Alternate Function Select, Pull-Up Resistor, Pull-Down Resistor,
// and Digital Enable are all locked for PF0 until a value of 0x4C4F434B
// is written to the Port F GPIO Lock Register.  After Port F is
// unlocked, bit 0 of the Port F GPIO Commit Register must be set to
// allow access to PF0's control registers.  On the LM4F120, the other
// bits of the Port F GPIO Commit Register are hard-wired to 1, meaning
// that the rest of Port F can always be freely re-configured at any
// time.  Requiring this procedure makes it unlikely to accidentally
// re-configure the JTAG and NMI pins as GPIO, which can lock the
// debugger out of the processor and make it permanently unable to be
// debugged or re-programmed.
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"

#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define PF0                     (*((volatile uint32_t *)0x40025004))
#define PF4                     (*((volatile uint32_t *)0x40025040))
#define PA5                     (*((volatile uint32_t *)0x40004080))
#define SWITCHES                (*((volatile uint32_t *)0x40025044))
#define SW1       0x10                      // on the left side of the Launchpad board
#define SW2       0x01                      // on the right side of the Launchpad board
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control

//------------Switch_Init------------
// Initialize GPIO Port A bit 5 for input
// Input: none
// Output: none
void Switch_Init(void){ 
  SysTick_Init();
  SYSCTL_RCGCGPIO_R |= 0x00000001;     // 1) activate clock for Port A
  while((SYSCTL_PRGPIO_R&0x01) == 0){};// ready?
  GPIO_PORTA_AMSEL_R &= ~0x20;      // 3) disable analog on PA5
  GPIO_PORTA_PCTL_R &= ~0x00F00000; // 4) PCTL GPIO on PA5
  GPIO_PORTA_DIR_R &= ~0x20;        // 5) direction PA5 input
  GPIO_PORTA_AFSEL_R &= ~0x20;      // 6) PA5 regular port function
  GPIO_PORTA_DEN_R |= 0x20;         // 7) enable PA5 digital port
}
//------------Switch_Input------------
// Read and return the status of GPIO Port A bit 5 
// Input: none
// Output: 0x20 if PA5 is high
//         0x00 if PA5 is low
uint32_t Switch_Input(void){
  return PA5; // return 0x20(pressed) or 0(not pressed)
}
uint32_t Switch_Input2(void){
  return (GPIO_PORTA_DATA_R&0x20); // 0x20(pressed) or 0(not pressed)
}

//------------Board_Init------------
// Initialize GPIO Port F for negative logic switches on PF0 and
// PF4 as the Launchpad is wired.  Weak internal pull-up
// resistors are enabled, and the NMI functionality on PF0 is
// disabled.
// Input: none
// Output: none
void Board_Init(void){            
  SYSCTL_RCGCGPIO_R |= 0x20;     // 1) activate Port F
  while((SYSCTL_PRGPIO_R&0x20) == 0){};// ready?
                                 // 2a) unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R |= (SW1|SW2);  // 2b) enable commit for PF4 and PF0
                                 // 3) disable analog functionality on PF4 and PF0
  GPIO_PORTF_AMSEL_R &= ~(SW1|SW2);
                                 // 4) configure PF0 and PF4 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF0FFF0)+0x00000000;
  GPIO_PORTF_DIR_R &= ~(SW1|SW2);// 5) make PF0 and PF4 in (built-in buttons)
                                 // 6) disable alt funct on PF0 and PF4
  GPIO_PORTF_AFSEL_R &= ~(SW1|SW2);
//  delay = SYSCTL_RCGC2_R;        // put a delay here if you are seeing erroneous NMI
  GPIO_PORTF_PUR_R |= (SW1|SW2); // enable weak pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R |= (SW1|SW2); // 7) enable digital I/O on PF0 and PF4
}

//------------Board_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 0x01 if only Switch 1 is pressed
//         0x10 if only Switch 2 is pressed
//         0x00 if both switches are pressed
//         0x11 if no switches are pressed
uint32_t Board_Input(void){
  return SWITCHES;
}

// Program 2.9 from Volume 2
#define PB1 (*((volatile uint32_t *)0x40005008))
//------------Switch_Init3------------
// Initialize GPIO Port B bit 1 for input
// Input: none
// Output: none
void Switch_Init3(void){
  SYSCTL_RCGCGPIO_R |= 0x02;        // 1) activate clock for Port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};// ready?
  GPIO_PORTB_DIR_R &= ~0x02;        // PB1 is an input
  GPIO_PORTB_AFSEL_R &= ~0x02;      // regular port function
  GPIO_PORTB_AMSEL_R &= ~0x02;      // disable analog on PB1 
  GPIO_PORTB_PCTL_R &= ~0x000000F0; // PCTL GPIO on PB1 
  GPIO_PORTB_DEN_R |= 0x02;         // PB3-0 enabled as a digital port
}
//------------Switch_Input3------------
// Read and return the status of GPIO Port B bit 1 
// Input: none
// Output: 0x02 if PB1 is high
//         0x00 if PB1 is low
uint32_t Switch_Input3(void){ 
  return PB1;      // 0x02 if pressed, 0x00 if not pressed
}

#define DELAY10MS 160000
#define DELAY10US 160
//------------Switch_Debounce------------
// Read and return the status of the switch 
// Input: none
// Output: 0x02 if PB1 is high
//         0x00 if PB1 is low
// debounces switch
uint32_t Switch_Debounce(void){
uint32_t in,old,time; 
  time = 1000; // 10 ms
  old = Switch_Input();
  while(time){
    SysTick_Wait(DELAY10US); // 10us
    in = Switch_Input();
    if(in == old){
      time--; // same value 
    }else{
      time = 1000;  // different
      old = in;
    }
  }
  return old;
}

//------------Switch_Debounce------------
// wait for the switch to be touched 
// Input: none
// Output: none
// debounces switch
void Switch_WaitForTouch(void){
// wait for release
  while(Switch_Input()){};
  SysTick_Wait(DELAY10MS); // 10ms
// wait for touch
  while(Switch_Input()==0){};
  SysTick_Wait(800000); // 10ms
}
  
