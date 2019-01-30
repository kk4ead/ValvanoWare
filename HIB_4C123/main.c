// main.c
// Runs on TM4C123
// Test the hiberation functions. Toggles blue LED every 5 seconds
// Daniel and Jonathan Valvano
// July 8, 2015

//********************WARNING*********************
// This program will enable hibernation, which disables the debugger and reset.
// In order to reprogram the board you will need to run unlock using LM Flash Programmer.
// If you do not know how to run unlock, then do not download this example
//********************WARNING*********************

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

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

// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "HIB.h"
#include "SysTick.h"

#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025030))

int main1(void){ 
  SysTick_Init();                   // running at 16 MHz
  SYSCTL_RCGCGPIO_R |= 0x00000020;  // activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){};
  GPIO_PORTF_DIR_R = 0x0E;          // PF3-1 out
  GPIO_PORTF_DEN_R = 0x0E;          // enable digital I/O on PF3-1
  while(1){
    SysTick_Wait10ms(500); // 5 second wait
    PF2 ^= 0x04;           // toggle blue
  }
}
// on wakeup from hibernation, it will restart main
int main(void){ 
uint32_t count;
  HIB_Init();
  SYSCTL_RCGCGPIO_R |= 0x00000020;  // activate clock for Port F
  while((SYSCTL_PRGPIO_R&0x20)==0){};
  GPIO_PORTF_DIR_R = 0x0E;          // PF3-1 out
  GPIO_PORTF_DEN_R = 0x0E;          // enable digital I/O on PF3-1
  PF2 = 0x04;
  count = HIB_GetData();
  count++;  // number of times awakened 
  HIB_SetData(count);
  if(count&0x01){
    PF2 = 0x04; // toggle blue every 5 sec
  }else{
    PF2 = 0x00;
  }
  HIB_Hiberate(6); // 5 second sleep
// should not return here
  PF1 = 0x02;           // set red
  while(1){
  }
}
// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06
