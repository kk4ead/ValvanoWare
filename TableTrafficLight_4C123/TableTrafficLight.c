// TableTrafficLight.c
// Runs on LM4F120/TM4C123
// Use a table implementation of a Moore finite state machine to operate
// a traffic light.
// Daniel Valvano
// February 2, 2015

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

// north facing car detector connected to PE1 (1=car present)
// east facing car detector connected to PE0 (1=car present)
// east facing red light connected to PB5
// east facing yellow light connected to PB4
// east facing green light connected to PB3
// north facing red light connected to PB2
// north facing yellow light connected to PB1
// north facing green light connected to PB0

#include <stdint.h>
#include "PLL.h"
#include "SysTick.h"

#define LIGHT                   (*((volatile uint32_t *)0x400050FC))
#define GPIO_PORTB_DIR_R        (*((volatile uint32_t *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile uint32_t *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile uint32_t *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile uint32_t *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile uint32_t *)0x4000552C))
#define SENSOR                  (*((volatile uint32_t *)0x4002400C))
#define GPIO_PORTE_DIR_R        (*((volatile uint32_t *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile uint32_t *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile uint32_t *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile uint32_t *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile uint32_t *)0x4002452C))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))

struct State {
  uint32_t Out;      // 6-bit output
  uint32_t Time;     // 10 ms
  uint8_t Next[4];}; // depends on 2-bit input
typedef const struct State STyp;
#define goN   0
#define waitN 1
#define goE   2
#define waitE 3
STyp FSM[4]={
 {0x21,3000,{goN,waitN,goN,waitN}},
 {0x22, 500,{goE,goE,goE,goE}},
 {0x0C,3000,{goE,goE,waitE,waitE}},
 {0x14, 500,{goN,goN,goN,goN}}};

int main(void){
  uint8_t n; // state number
  uint32_t Input;
  PLL_Init(Bus80MHz);               // initialize 50 MHz system clock
  SysTick_Init();                   // initialize SysTick timer
  SYSCTL_RCGCGPIO_R |= 0x12;        // activate clock for Port E and Port B
  // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&0x12) == 0){};
  GPIO_PORTB_DIR_R |= 0x3F;         // make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F;      // disable alt funct on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;         // enable digital I/O on PB5-0
                                    // configure PB5-0 as GPIO
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFF000000)+0x00000000;
  GPIO_PORTB_AMSEL_R &= ~0x3F;      // disable analog functionality on PB5-0
  GPIO_PORTE_DIR_R &= ~0x03;        // make PE1-0 in
  GPIO_PORTE_AFSEL_R &= ~0x03;      // disable alt funct on PE1-0
  GPIO_PORTE_DEN_R |= 0x03;         // enable digital I/O on PE1-0
                                    // configure PE1-0 as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTE_AMSEL_R &= ~0x03;      // disable analog functionality on PE1-0
  n = goN;                          // initial state: Green north; Red east
  while(1){
    LIGHT = FSM[n].Out;             // set lights to current state's Out value
    SysTick_Wait10ms(FSM[n].Time);  // wait 10 ms * current state's Time value
    Input = SENSOR;                 // get new input from car detectors
    n = FSM[n].Next[Input];         // transition to next state
  }
}
