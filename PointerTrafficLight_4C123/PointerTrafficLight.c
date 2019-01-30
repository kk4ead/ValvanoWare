// PointerTrafficLight.c
// Runs on LM4F120/TM4C123
// Use a pointer implementation of a Moore finite state machine to operate
// a traffic light.
// Daniel Valvano
// May 5, 2015
/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1 Program 6.8, Example 6.4
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Volume 2 Program 3.1, Example 3.1

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

// east facing red light connected to PB5
// east facing yellow light connected to PB4
// east facing green light connected to PB3
// north facing red light connected to PB2
// north facing yellow light connected to PB1
// north facing green light connected to PB0
// north facing car detector connected to PE1 (1=car present)
// east facing car detector connected to PE0 (1=car present)

#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"
#include "../inc/tm4c123gh6pm.h"

#define LIGHT                   (*((volatile uint32_t *)0x400050FC))
#define GPIO_PORTB_OUT          (*((volatile uint32_t *)0x400050FC)) // bits 5-0
#define GPIO_PORTE_IN           (*((volatile uint32_t *)0x4002400C)) // bits 1-0
#define SENSOR                  (*((volatile uint32_t *)0x4002400C))

struct State {
  uint32_t Out;            // 6-bit output
  uint32_t Time;           // 10 ms
  const struct State *Next[4];};// depends on 2-bit input
typedef const struct State STyp;
#define goN   &FSM[0]
#define waitN &FSM[1]
#define goE   &FSM[2]
#define waitE &FSM[3]
STyp FSM[4]={
 {0x21,300,{goN,waitN,goN,waitN}},
 {0x22, 50,{goE,goE,goE,goE}},
 {0x0C,300,{goE,goE,waitE,waitE}},
 {0x14, 50,{goN,goN,goN,goN}}};

int main(void){
  STyp *Pt;  // state pointer
  uint32_t Input;
  volatile uint32_t delay;
  PLL_Init(Bus80MHz);          // configure for 50 MHz clock
  SysTick_Init();              // initialize SysTick timer
  // activate port B and port E
  SYSCTL_RCGCGPIO_R |= 0x12;
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTB_DIR_R |= 0x3F;    // make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F; // disable alt func on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;    // enable digital I/O on PB5-0
                               // configure PB5-0 as GPIO
  GPIO_PORTB_PCTL_R &= ~0x00FFFFFF;
  GPIO_PORTB_AMSEL_R &= ~0x3F; // disable analog functionality on PB5-0
  GPIO_PORTE_DIR_R &= ~0x03;   // make PE1-0 in
  GPIO_PORTE_AFSEL_R &= ~0x03; // disable alt func on PE1-0
  GPIO_PORTE_DEN_R |= 0x03;    // enable digital I/O on PE1-0
                               // configure PE1-0 as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTE_AMSEL_R &= ~0x03; // disable analog functionality on PE1-0
  Pt = goN;                    // initial state: Green north; Red east
  while(1){
    LIGHT = Pt->Out;           // set lights to current state's Out value
    SysTick_Wait10ms(Pt->Time);// wait 10 ms * current state's Time value
    Input = SENSOR;            // get new input from car detectors
    Pt = Pt->Next[Input];      // transition to next state
  }
}
