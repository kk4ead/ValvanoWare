// Stepper.h
// Runs on LM4F120/TM4C123
// Provide functions that step the motor once clockwise, step
// once counterclockwise, and initialize the stepper motor
// interface.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Example 4.1, Programs 4.4, 4.5, and 4.6
   Hardware circuit diagram Figure 4.27

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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


// Initialize Stepper interface
void Stepper_Init(void);

// Move 1.8 degrees clockwise, 
// delay is the time to wait after each step in bus cycles
void Stepper_CW(uint32_t delay);

// Move 1.8 degrees counterclockwise,
// delay is the time to wait after each step in bus cycles
void Stepper_CCW(uint32_t delay);

// Turn stepper motor to desired position
// (0 <= desired <= 199)
// time is the number of bus cycles to wait after each step
void Stepper_Seek(uint8_t desired, uint32_t time);

