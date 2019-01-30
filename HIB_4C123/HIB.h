// HIB.h
// Runs on TM4C123
// Hiberation module for deep sleep.
// Jonathan Valvano
// July 8, 2015

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


// *******HIB_Init*************
// Initialize RTC and Hiberation module
// Inputs: none
// Outputs: none
void HIB_Init(void);

// *******HIB_Hiberate*************
// Start RTC and Hiberation, go to sleep
// Inputs:  seconds is the time to sleep
// Outputs: none
void HIB_Hiberate(uint32_t seconds);

// *******HIB_SetData*************
// save data in Hibernation module
// Inputs:  data is important information to retain
// Outputs: data returns the same data entered
void HIB_SetData(uint32_t data);

// *******HIB_GetData*************
// retrieve data from Hibernation module
// Inputs:  none
// Outputs: data returns the same data entered
uint32_t HIB_GetData(void);


