// GPIO.c
// Runs on LM4F120/TM4C123
// Initialize four GPIO pins as outputs.  Continually generate output to
// drive simulated stepper motor.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
  Volume 1 Program 4.5

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Volume 2 Example 2.3, Program 2.11, Figure 2.32

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

// PD3 is an output to LED3, negative logic
// PD2 is an output to LED2, negative logic
// PD1 is an output to LED1, negative logic
// PD0 is an output to LED0, negative logic

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"

#define LEDS (*((volatile uint32_t *)0x4000703C))
// access PD3-PD0

void GPIO_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x08;        // 1) activate port D
  while((SYSCTL_PRGPIO_R&0x08)==0){};   // allow time for clock to stabilize
                                    // 2) no need to unlock PD3-0
  GPIO_PORTD_AMSEL_R &= ~0x0F;      // 3) disable analog functionality on PD3-0
  GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) GPIO
  GPIO_PORTD_DIR_R |= 0x0F;         // 5) make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;      // 6) regular port function
  GPIO_PORTD_DEN_R |= 0x0F;         // 7) enable digital I/O on PD3-0
}
int main1(void){
  GPIO_Init();
  while(1){
    LEDS = 10; // 1010, LED is 0101
    LEDS = 9;  // 1001, LED is 0110
    LEDS = 5;  // 0101, LED is 1010
    LEDS = 6;  // 0110, LED is 1001
  }
}


// Program 2.11 from Volume 2
#define STEPPER  (*((volatile uint32_t *)0x4000703C))
static void step(uint32_t n){
  STEPPER = n;          // output to stepper causing it to step once
  SysTick_Wait10ms(10); // program 2.10
}
int main(void){ // reset clears AFSEL, PCTL, AMSEL
  SysTick_Init();
  SYSCTL_RCGCGPIO_R |= 0x08;        // activate clock for Port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// ready?
  GPIO_PORTD_DIR_R |= 0x0F;         // PD3-0 is an output
  GPIO_PORTD_DEN_R |= 0x0F;         // PD3-0 enabled as a digital port
  while(1){
    step(5);  // motor is 0101
    step(6);  // motor is 0110
    step(10); // motor is 1010
    step(9);  // motor is 1001
  }
}
