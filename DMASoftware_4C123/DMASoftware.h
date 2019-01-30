// DMASoftware.h
// Runs on LM4F120/TM4C123
// Software triggered memory block transfer
// Jonathan Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2014
   Section 6.4.5, Program 6.1

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



// ************DMA_Init*****************
// Initialize the memory to memory transfer
// This needs to be called once before requesting a transfer
// Inputs:  none
// Outputs: none
void DMA_Init(void);

// ************DMA_Transfer*****************
// Called to transfer 32-bit words from source to destination
// Inputs:  source is a pointer to the first 32-bit word of the original data
//          destination is a pointer to a place to put the copy
//          count is the number of words to transfer (max is 1024 words)
// Outputs: none
// This routine does not wait for completion
void DMA_Transfer(uint32_t *source, uint32_t *destination, uint32_t count);

// ************DMA_Status*****************
// Can be used to check the status of a previous request
// Inputs:  none
// Outputs: true if still active, false if complete
// This routine does not wait for completion
uint32_t DMA_Status(void); 
