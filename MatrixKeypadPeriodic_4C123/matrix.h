// Matrix.h
// Runs on  LM4F120/TM4C123
// Provide functions that initialize GPIO ports and SysTick 
// Use periodic polling
// Daniel Valvano
// August 11, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

   Example 5.4, Figure 5.18, Program 5.13

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

// PA5 connected to column 3 (keypad pin 4) using 10K pull-up
// PA4 connected to column 2 (keypad pin 3) using 10K pull-up
// PA3 connected to column 1 (keypad pin 2) using 10K pull-up
// PA2 connected to column 0 (keypad pin 1) using 10K pull-up
// PD3 connected to row 3 (keypad pin 8)
// PD2 connected to row 2 (keypad pin 7)
// PD1 connected to row 1 (keypad pin 6) (remove R9, R10)
// PD0 connected to row 0 (keypad pin 5) (remove R9, R10)

// [1] [2] [3] [A]
// [4] [5] [6] [B]
// [7] [8] [9] [C]
// [*] [0] [#] [D]
// Pin1 . . . . . . . . Pin8
// Pin 1 -> Column 0 (column starting with 1)
// Pin 2 -> Column 1 (column starting with 2)
// Pin 3 -> Column 2 (column starting with 3)
// Pin 4 -> Column 3 (column starting with A)
// Pin 5 -> Row 0 (row starting with 1)
// Pin 6 -> Row 1 (row starting with 4)
// Pin 7 -> Row 2 (row starting with 7)
// Pin 8 -> Row 3 (row starting with *)


// Initialization of Matrix keypad
void Matrix_Init(void);
       
// input ASCII character from keypad
// spin if Fifo is empty
char Matrix_InChar(void);


