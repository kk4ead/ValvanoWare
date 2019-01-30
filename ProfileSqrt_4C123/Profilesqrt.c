// Profilesqrt.c
// Runs on LM4F120/TM4C123
// Daniel Valvano
// February 9, 2015

/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Volume 2, Programs 3.16-3.18

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

// PF2 is an output for debugging
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}
// Newton's method
// s is an integer
// sqrt(s) is an integer
uint32_t sqrt(uint32_t s){
uint32_t t;   // t*t will become s
int n;             // loop counter
  t = s/16+1;      // initial guess
  for(n = 16; n; --n){ // will finish
    t = ((t*t+s)/t)/2;
  }
  return t;
}
uint32_t before,elapsed,ss,tt;
int main(void){
  SYSCTL_RCGCGPIO_R |= 0x20; // activate Port F
  SysTick_Init();            // initialize SysTick timer, Program 4.7
  GPIO_PORTF_DIR_R |= 0x04;  // make PF2 out (built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;// disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;  // enable digital I/O on PF2
                             // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;    // disable analog functionality on PF
  ss = 230400;
  before = NVIC_ST_CURRENT_R;
  tt = sqrt(ss);
  elapsed = (before - NVIC_ST_CURRENT_R - 4)&0x00FFFFFF;
  // the number 4 depends on the instructions before and after test
  // if you remove the call to sqrt, elapsed measures 0

  while(1){
    ss = 230400;
    GPIO_PORTF_DATA_R = 0x04; //  PF2=1
    tt = sqrt(ss);
    GPIO_PORTF_DATA_R = 0x00; //  PF2=0
  }
}

#define PROFILE (*((volatile uint32_t *)0x4000703C))
uint32_t sqrt2(uint32_t s){
uint32_t t;  // t*t becomes s
int n;            // loop counter 
  PROFILE = 1;
  t = s/10+1;     // initial guess 
  PROFILE = 2;
  for(n = 16; n; --n){  // will finish
    PROFILE = 4;
    t = ((t*t+s)/t)/2;  
    PROFILE = 8;
  }
  PROFILE = 0;
  return t; 
}  

int main2(void){
  SYSCTL_RCGCGPIO_R |= 0x08; // activate Port D
  while((SYSCTL_PRGPIO_R&0x08) ==0){};
  SysTick_Init();            // initialize SysTick timer, Program 4.7
  GPIO_PORTD_DIR_R |= 0x0F;  // make PD3-0 out (logic analyzer)
  GPIO_PORTD_DEN_R |= 0x0F;  // enable digital I/O on PD3-0 out
  while(1){
    ss = 230400;
    tt = sqrt2(ss);
  }
}
