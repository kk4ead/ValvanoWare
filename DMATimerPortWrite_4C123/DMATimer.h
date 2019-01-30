// DMATimer.h
// Runs on LM4F120/TM4C123
// Periodic timer triggered DMA transfer
// source: One large byte-aligned memory
//   BlockSize is number of bytes in a block (<=1024)
//   NumBlocks is the number if blocks
//   Memory buffer size is BlockSize*NumBlocks
// destination: Port
//   initialized in main program

// Uses Timer5A to trigger the DMA,
//   read from a large memory Buffer, and
//   then write to an 8-bit PORT
// There is a Timer5A interrupt after the buffer has been transferred
//   In ping-pong mode the ISR sets up the next buffer

// Usage
// 0) configure a GPIO port as output, can use bit-specific addressing
// 1) call DMA_Init once, setting the output period (12.ns) and port address
// 2) call DMA_Transfer to initiate DMA specifying
//    memory buffer address, byte aligned
//    block size and number of blocks
// Jonathan Valvano
// May 2, 2015


/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Section 6.4.5

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
// Main program must initialize used for destination
// Source address will be set in DMA_Transfer
// DMA triggered by timer 5A every period bus cycles
// This function needs to be called once before requesting a transfer
// The destination address is fixed, which can be a bit specific addressing
// The source address is incremented each byte
// Inputs:  period in 12.5nsec
//          destination address of output port
// Outputs: none
void DMA_Init(uint16_t period, uint32_t *destination);

// ************DMA_Transfer*****************
// Called to transfer bytes from source to destination
// The source address is incremented each byte, destination address is fixed
// Inputs:  source is a pointer to the memory buffer containing 8-bit data
//          blocksize is the size of each block (max is 2048 bytes) (must be even)
//          numblocks is the number of blocks
// Outputs: none
// This routine does not wait for completion
void DMA_Transfer(uint8_t *source, uint32_t blocksize,
                   uint32_t numblocks);

// ************DMA_Stop*****************
// Stop the transfer bytes from source to destination
// Inputs:  none
// Outputs: none
void DMA_Stop(void);

// ************DMA_Status*****************
// Return the status of the DMA Channel 8 module in an
// enumerated type.
// Inputs:  none
// Outputs: enumerated type status of the DMA Channel 8
enum ch8status{
  IDLE = 0,           // ready to start a new transfer
  BUSY = 1,           // transfer is pending or in progress
  UNINITIALIZED = 2   // DMA_Init() has not been called
};
enum ch8status DMA_Status(void);
