// MAX549.h
// Runs on LM4F120/TM4C123
// Use SSI0 to send a 16-bit code to the MAX539 and return
// the reply.
// Daniel Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
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

// Max549 pin 1 ground
// Max549 pin 2 OUTA
// Max549 pin 3 CS, SSI0Fss connected to PA3
// Max549 pin 4 DIN, SSI0Tx connected to PA5
// Max549 pin 5 SCLK SSI0Clk connected to PA2
// Max549 pin 6 OUTB
// Max549 pin 7 REF (cheap solution connects this to +3.3V)
// Max549 pin 8 +3.3V

//********DAC_Init*****************
// Initialize Max539 dual 8-bit DAC
// inputs:  initial voltage output (0 to 255)
// outputs: none
// assumes: system clock rate less than 20 MHz
void DAC_Init(uint8_t data);

//********DAC_Out*****************
// Send data to Max539 dual 8-bit DAC
// inputs:  two voltage outputs (0 to 255)
// outputs: none
void DAC_Out(uint8_t outA, uint8_t outB);
