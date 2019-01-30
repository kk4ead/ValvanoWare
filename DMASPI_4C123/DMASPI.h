// DMASPI.h
// Runs on LM4F120/TM4C123
// Periodic timer triggered DMA transfer
// Uses Timer5A to trigger the DMA, read from a 16-bit buffer, and then write to a fixed port address
// There is a Timer5A interrupt after the buffer has been transferred. 
// This example runs continuously, outputing the buffer over and over.
// Jonathan Valvano
// May 2, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Section 6.4.5, Program 6.1

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

// ************DMA_Init*****************
// Initialize the buffer to port transfer, triggered by timer 5A
// This needs to be called once before requesting a transfer
// The source address increments by 2, destination address is fixed
// Call DMA_Start to begin continuous transfer
// Call DMA_Stop to halt the transfer
// Inputs:  period in 12.5nsec
// Outputs: none
void DMA_Init(uint16_t period);

// ************DMA_Start*****************
// Called to transfer halfwords from source to destination
// The source address is incremented by two each 16-bit transfer, destination address is fixed
// Inputs:  source is a pointer to a RAM buffer containing waveform to output
//          destination is a pointer to 32-bit DAC device (SSI0_DR_R), least significant 16 bits
//          count is the number of halfwords to transfer (max is 1024 halfwords)
// Outputs: none
// This routine does not wait for completion, runs continuously
void DMA_Start(uint16_t *source, volatile uint32_t *destination, uint32_t  count);

// ************DMA_Status*****************
// Can be used to check the status of the continuous DMA transfer
// Inputs:  none
// Outputs: the number of buffers transferred
uint32_t  DMA_Status(void);

// ************DMA_Stop*****************
// Stop the transfer halfwords from source to destination
// Inputs:  none
// Outputs: none
void DMA_Stop(void);
