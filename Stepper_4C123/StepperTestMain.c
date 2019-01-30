// StepperTestMain.c
// Runs on LM4F120/TM4C123
// Test the functions provided by Stepper.c,
// 
// Before connecting a real stepper motor, remember to put the
// proper amount of delay between each CW() or CCW() step.
// Daniel Valvano
// September 12, 2013

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

// PD3 connected to driver for stepper motor coil A
// PD2 connected to driver for stepper motor coil A'
// PD1 connected to driver for stepper motor coil B
// PD0 connected to driver for stepper motor coil B'
#include <stdint.h>
#include "stepper.h"
#define T1ms 16000    // assumes using 16 MHz PIOSC (default setting for clock source)
int main(void){
  Stepper_Init();
  Stepper_CW(T1ms);   // Pos=1; GPIO_PORTD_DATA_R=9
  Stepper_CW(T1ms);   // Pos=2; GPIO_PORTD_DATA_R=5
  Stepper_CW(T1ms);   // Pos=3; GPIO_PORTD_DATA_R=6
  Stepper_CW(T1ms);   // Pos=4; GPIO_PORTD_DATA_R=10
  Stepper_CW(T1ms);   // Pos=5; GPIO_PORTD_DATA_R=9
  Stepper_CW(T1ms);   // Pos=6; GPIO_PORTD_DATA_R=5
  Stepper_CW(T1ms);   // Pos=7; GPIO_PORTD_DATA_R=6
  Stepper_CW(T1ms);   // Pos=8; GPIO_PORTD_DATA_R=10
  Stepper_CW(T1ms);   // Pos=9; GPIO_PORTD_DATA_R=9
  Stepper_CCW(T1ms);  // Pos=8; GPIO_PORTD_DATA_R=10
  Stepper_CCW(T1ms);  // Pos=7; GPIO_PORTD_DATA_R=6
  Stepper_CCW(T1ms);  // Pos=6; GPIO_PORTD_DATA_R=5
  Stepper_CCW(T1ms);  // Pos=5; GPIO_PORTD_DATA_R=9
  Stepper_CCW(T1ms);  // Pos=4; GPIO_PORTD_DATA_R=10
  Stepper_Seek(8,T1ms);// Pos=8; GPIO_PORTD_DATA_R=10
  Stepper_Seek(0,T1ms);// Pos=0; GPIO_PORTD_DATA_R=10
  while(1){
    Stepper_CW(10*T1ms);   // output every 10ms
  }
}
