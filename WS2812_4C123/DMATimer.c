// DMATimer.c
// Runs on LM4F120/TM4C123
// Periodic timer triggered DMA transfer
// source: One large byte-aligned memory
//   BlockSize is number of bytes in a block (<=1024)
//   NumBlocks is the number of blocks
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
// August 19, 2014

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

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "DMATimer.h"

// functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);  // previous I bit, disable interrupts
void EndCritical(int32_t sr); // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// The control table used by the uDMA controller.  This table must be aligned to a 1024 byte boundary.
// really two 128-word tables: primary and alternate
// there are 32 channels, we are using channel 8, encoding 3
// each channel has 4 words in the primary table, and another 4 words in the second
uint32_t ucControlTable[256] __attribute__ ((aligned(1024)));
// Timer5A uses uDMA channel 8 encoding 3
#define CH8 (8*4)
#define CH8ALT (8*4+128)

#define BIT8 0x00000100
uint8_t *SourcePt;                 // last address of the source buffer, incremented by 1
volatile uint32_t *DestinationPt;  // fixed address, Port to output
uint32_t BlockSize;                // number of bytes to transmit in each block
uint32_t NumBlocks;                // number of blocks to transmit
uint32_t NumberOfBuffersSent=0;
enum ch8status Status = UNINITIALIZED;

// ***************** Timer5A_Init ****************
// Activate Timer5A trigger DMA periodically
// Inputs:  period in 12.5nsec
// Outputs: none
void Timer5A_Init(uint16_t period){
  SYSCTL_RCGCTIMER_R |= 0x20;      // 0) activate timer5
  while((SYSCTL_PRTIMER_R&SYSCTL_PRTIMER_R5) == 0){};
  TIMER5_CTL_R &= ~0x00000001;     // 1) disable timer5A during setup
  TIMER5_CFG_R = 0x00000004;       // 2) configure for 16-bit timer mode
  TIMER5_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
  TIMER5_TAILR_R = period-1;       // 4) reload value
  TIMER5_TAPR_R = 0;               // 5) 12.5ns timer5A
  TIMER5_ICR_R = 0x00000001;       // 6) clear timer5A timeout flag
  TIMER5_IMR_R |= 0x00000001;      // 7) arm timeout interrupt
  NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|0x00000000; // 8) priority 0
  NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 92 in NVIC
  // vector number 108, interrupt number 92
/*  TIMER5_CTL_R |= 0x00000001;      // 10) enable timer5A when DMA transfer initiates*/
// interrupts enabled in the main program after all devices initialized
}

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
void DMA_Init(uint16_t period, uint32_t *destination){
  int i;
  DestinationPt = destination;

  for(i=0; i<256; i++){
    ucControlTable[i] = 0;
  }
  SYSCTL_RCGCDMA_R = 0x01;    // µDMA Module Run Mode Clock Gating Control
                              // allow time to finish
  while((SYSCTL_PRDMA_R&SYSCTL_PRDMA_R0) == 0){};
  UDMA_CFG_R = 0x01;          // MASTEN Controller Master Enable
  UDMA_CTLBASE_R = (uint32_t)ucControlTable;
  UDMA_CHMAP1_R = (UDMA_CHMAP1_R&0xFFFFFFF0)|0x00000003;  // timer5A, channel 8, encoding 3
  UDMA_PRIOSET_R |= BIT8;     // use high priority
  UDMA_ALTCLR_R = BIT8;       // use primary control initially
  UDMA_USEBURSTCLR_R = BIT8;  // responds to both burst and single requests
  UDMA_REQMASKCLR_R = BIT8;   // allow the µDMA controller to recognize requests for this channel
  Timer5A_Init(period);
  Status = IDLE;
}

// private function used to reprogram primary channel control structure
void static setRegular(void){
  ucControlTable[CH8]   = (uint32_t)SourcePt+BlockSize-1;  // last address of memory
  ucControlTable[CH8+1] = (uint32_t)DestinationPt;         // fixed address (when DSTINC is 0x3)
  ucControlTable[CH8+2] = 0xC0000003+((BlockSize-1)<<4);   // DMA Channel Control Word (DMACHCTL)
/* DMACHCTL          Bits    Value Description
   DSTINC            31:30   3     8-bit destination address no increment
   DSTSIZE           29:28   0     8-bit destination data size
   SRCINC            27:26   0     1 byte source address increment
   SRCSIZE           25:24   0     8-bit source data size
   reserved          23:18   0     Reserved
   ARBSIZE           17:14   0     Arbitrates after 1 transfer
   XFERSIZE          13:4  count-1 Transfer count items
   NXTUSEBURST       3       0     N/A for this transfer type
   XFERMODE          2:0     011   Use ping-pong transfer mode
  */
//  UDMA_ENASET_R |= BIT8;  // µDMA Channel 8 is enabled.
  // bit 8 in UDMA_ENASET_R become clear when done
  // bits 2:0 ucControlTable[CH8+2] become clear when done
  SourcePt = SourcePt+BlockSize;
}
// private function used to reprogram alternate channel control structure
void static setAlternate(void){
  ucControlTable[CH8ALT]   = (uint32_t)SourcePt+BlockSize-1;  // last address of memory
  ucControlTable[CH8ALT+1] = (uint32_t)DestinationPt;         // fixed address (when DSTINC is 0x3)
  ucControlTable[CH8ALT+2] = 0xC0000003+((BlockSize-1)<<4);   // DMA Channel Control Word (DMACHCTL)
/* DMACHCTL          Bits    Value Description
   DSTINC            31:30   11    no destination address increment
   DSTSIZE           29:28   00    8-bit destination data size
   SRCINC            27:26   00    8-bit source address increment
   SRCSIZE           25:24   00    8-bit source data size
   reserved          23:18   0     Reserved
   ARBSIZE           17:14   0     Arbitrates after 1 transfer
   XFERSIZE          13:4  count-1 Transfer count items
   NXTUSEBURST       3       0     N/A for this transfer type
   XFERMODE          2:0     011   Use ping-pong transfer mode
  */
  SourcePt = SourcePt+BlockSize;
}
// ************DMA_Transfer*****************
// Called to transfer bytes from source to destination
// The source address is incremented each byte, destination address is fixed
// Inputs:  source is a pointer to the memory buffer containing 8-bit data
//          blocksize is the size of each block (max is 2048 bytes) (must be even)
//          numblocks is the number of blocks
// Outputs: none
// This routine does not wait for completion
void DMA_Transfer(uint8_t *source, uint32_t blocksize,
                   uint32_t numblocks){
  DisableInterrupts();    // some or all of this initialization is a critical section
  Status = BUSY;
  BlockSize = blocksize/2;// number of bytes to transmit
  NumBlocks = 2*numblocks;
  NumberOfBuffersSent = 0;
  SourcePt = source;      // address of source buffer
  setRegular();
  setAlternate();
  NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 92 in NVIC
  // vector number 108, interrupt number 92
  TIMER5_CTL_R |= 0x00000001;      // 10) enable timer5A
  UDMA_ENASET_R |= BIT8;  // µDMA Channel 8 is enabled
  EnableInterrupts();     // re-enable interrupts and be ready for ping-pong interrupt
}

// ************DMA_Stop*****************
// Stop the transfer bytes from source to destination
// Inputs:  none
// Outputs: none
void DMA_Stop(void){
  UDMA_ENACLR_R = BIT8;        // µDMA Channel 8 is disabled
  NVIC_DIS2_R = 0x10000000;    // 9) disable interrupt 92 in NVIC
  TIMER5_CTL_R &= ~0x00000001; // 10) disable timer5A
  Status = IDLE;
}

void Timer5A_Handler(void){ // interrupts after each block is transferred
  TIMER5_ICR_R = TIMER_ICR_TATOCINT; // acknowledge timer5A timeout
  if((ucControlTable[CH8+2]&0x0007) == 0){     // regular buffer complete
    NumberOfBuffersSent++;                     // increment counter
    if(NumberOfBuffersSent < (NumBlocks - 1)){ // still more data to send
      setRegular();                            // rebuild channel control structure
    }
  }
  if((ucControlTable[CH8ALT+2]&0x0007) == 0){  // alternate buffer complete
    NumberOfBuffersSent++;                     // increment counter
    if(NumberOfBuffersSent < (NumBlocks - 1)){ // still more data to send
      setAlternate();                          // rebuild channel control structure
    }
  }
  if(((ucControlTable[CH8+2]&0x0007) == 0) &&
     ((ucControlTable[CH8ALT+2]&0x0007) == 0)){// both buffers complete and not re-enabled above
    DMA_Stop();
  }
}

// ************DMA_Status*****************
// Return the status of the DMA Channel 8 module in an
// enumerated type.
// Inputs:  none
// Outputs: enumerated type status of the DMA Channel 8
enum ch8status DMA_Status(void){
  return Status;
}

