// MAX5353.h
// Runs on LM4F120/TM4C123
// Use SSI0 to send a 16-bit code to the MAX5353.
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

// SSI0Clk (SCLK, pin 4) connected to PA2
// SSI0Fss (!CS, pin 2) connected to PA3
// SSI0Tx (DIN, pin 3) connected to PA5
// see Figure 7.19 for complete schematic

//********DAC_Init*****************
// Initialize Max5353 12-bit DAC
// inputs:  initial voltage output (0 to 4095)
// outputs: none
// assumes: system clock rate less than 20 MHz
void DAC_Init(uint16_t data);

//********DAC_Out*****************
// Send data to Max5353 12-bit DAC
// inputs:  voltage output (0 to 4095)
// outputs: none
void DAC_Out(uint16_t code);
