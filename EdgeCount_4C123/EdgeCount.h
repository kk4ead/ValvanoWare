// EdgeCount.c
// Runs on LM4F120/TM4C123
// Use Timer0A in edge count mode to count the rising edge on PB6.
// Jonathan Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 6.xx, Program 6.xx

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

// external signal connected to PB6 (T0CCP0) (count on rising edge)

//------------EdgeCount_Init------------
// Initialize Timer0A in edge count mode to increment Timer0A
// on the rising edge of PB6 (T0CCP0).
// Input: none
// Output: none
void EdgeCount_Init(void);

//------------EdgeCount_Result------------
// Count the number of rising edges since initialization.
// Input: none
// Output: 24-bit count of edges
uint32_t EdgeCount_Result(void);
