// Stepper.c
// Runs on LM4F120/TM4C123
// Provide functions that step the motor once clockwise, step
// once counterclockwise, and initialize the stepper motor
// interface.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 4.1, Programs 4.4, 4.5, and 4.6
   Hardware circuit diagram Figure 4.27

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

// PD3 connected to driver for stepper motor coil A
// PD2 connected to driver for stepper motor coil A'
// PD1 connected to driver for stepper motor coil B
// PD0 connected to driver for stepper motor coil B'

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "systick.h"
struct State{
  uint8_t Out;           // Output
  const struct State *Next[2]; // CW/CCW
};
typedef const struct State StateType;
typedef StateType *StatePtr;
#define clockwise 0        // Next index
#define counterclockwise 1 // Next index
StateType fsm[4]={
  {10,{&fsm[1],&fsm[3]}},
  { 9,{&fsm[2],&fsm[0]}},
  { 5,{&fsm[3],&fsm[1]}},
  { 6,{&fsm[0],&fsm[2]}}
};
uint8_t Pos;     // between 0 and 199
const struct State *Pt;// Current State

#define STEPPER  (*((volatile uint32_t *)0x4000703C))
// Move 1.8 degrees clockwise, delay is the time to wait after each step
void Stepper_CW(uint32_t delay){
  Pt = Pt->Next[clockwise];     // circular
  STEPPER = Pt->Out; // step motor
  if(Pos==199){      // shaft angle
    Pos = 0;         // reset
  }
  else{
    Pos++; // CW
  }
  SysTick_Wait(delay);
}
// Move 1.8 degrees counterclockwise, delay is wait after each step
void Stepper_CCW(uint32_t delay){
  Pt = Pt->Next[counterclockwise]; // circular
  STEPPER = Pt->Out; // step motor
  if(Pos==0){        // shaft angle
    Pos = 199;       // reset
  }
  else{
    Pos--; // CCW
  }
  SysTick_Wait(delay); // blind-cycle wait
}
// Initialize Stepper interface
void Stepper_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x08; // 1) activate port D
  SysTick_Init();
  Pos = 0;                   
  Pt = &fsm[0]; 
                                    // 2) no need to unlock PD3-0
  GPIO_PORTD_AMSEL_R &= ~0x0F;      // 3) disable analog functionality on PD3-0
  GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO configure PD3-0 as GPIO
  GPIO_PORTD_DIR_R |= 0x0F;   // 5) make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;// 6) disable alt funct on PD3-0
  GPIO_PORTD_DR8R_R |= 0x0F;  // enable 8 mA drive
  GPIO_PORTD_DEN_R |= 0x0F;   // 7) enable digital I/O on PD3-0 
}

// Turn stepper motor to desired position
// (0 <= desired <= 199)
// time is the number of bus cycles to wait after each step
void Stepper_Seek(uint8_t desired, uint32_t time){
short CWsteps;
  if((CWsteps = (desired-Pos))<0){
    CWsteps+=200;
  } // CW steps is 0 to 199
  if(CWsteps > 100){
    while(desired != Pos){
      Stepper_CCW(time);
    }
  }
  else{
    while(desired != Pos){
      Stepper_CW(time);
    }
  }
}

