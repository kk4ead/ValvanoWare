// StepperTestMain.c
// Runs on LM4F120/TM4C123
// FSM control of stepper
// Input operation
// 00    stop
// 01    clockwise 5,6,10,9,...
// 10    counterclockwise 5,9,10,6,...
// 11    stop
// Daniel Valvano
// Feb 1, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
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
// PD7 connected to switch input
// PD6 connected to switch input
// PD3 connected to driver for stepper motor coil A
// PD2 connected to driver for stepper motor coil A'
// PD1 connected to driver for stepper motor coil B
// PD0 connected to driver for stepper motor coil B'
#include <stdint.h>
#include "systick.h"
#include "pll.h"
#include "../inc/tm4c123gh6pm.h"
#define T1sec 100   
struct State{
  uint8_t Out;           
  uint32_t Delay; 
  const struct State *Next[4]; 
};
typedef const struct State StateType;
#define S5  &fsm[0]
#define S6  &fsm[1]
#define S10 &fsm[2]
#define S9  &fsm[3]
StateType fsm[4]={
  {5, T1sec, S5, S6, S9, S5},
  {6, T1sec, S9,S10, S5, S6},
  {10,T1sec,S10, S9, S6,S10},
  {9, T1sec, S9, S5,S10, S9}
};
const struct State *Pt;  // Current State
#define STEPPER  (*((volatile uint32_t *)0x4000703C))
#define INPUT  (*((volatile uint32_t *)0x40007300))
uint32_t Input;
int main(void){
  PLL_Init(Bus80MHz);      // Program 4.6
  SysTick_Init();          // Program 4.7
  SYSCTL_RCGC2_R |= 0x08;  // 1) port D clock enabled
  Pt = &fsm[0];               
  GPIO_PORTD_LOCK_R = 0x4C4F434B;  // unlock GPIO Port D
  GPIO_PORTD_CR_R = 0xFF;          // allow changes to PD7-0
  GPIO_PORTD_AMSEL_R &= ~0xCF;      // 3) disable analog function
  GPIO_PORTD_PCTL_R &= ~0xFF00FFFF; // 4) GPIO
  GPIO_PORTD_DIR_R |= 0x0F;   // 5) make PD3-0 out
  GPIO_PORTD_DIR_R &= ~0xC0;  //    make PD7-6 input
  GPIO_PORTD_AFSEL_R &= ~0xCF;// 6) disable alt func on PD7-6,3-0
  GPIO_PORTD_DR8R_R |= 0x0F;  // enable 8 mA drive on PD7-6,3-0
  GPIO_PORTD_DEN_R |= 0xCF;   // 7) enable digital I/O on PD7-6,3-0
  while(1){
    STEPPER = Pt->Out;           // Output
    SysTick_Wait10ms(Pt->Delay); // Wait
    Input = (INPUT>>6);          // 0,1,2,3
    Pt = Pt->Next[Input];        // Next   
  }
}
