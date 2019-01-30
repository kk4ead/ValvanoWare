// DMASoftware.c
// Runs on LM4F120/TM4C123
// Software triggered memory block transfer
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
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"


// The control table used by the uDMA controller.  This table must be aligned to a 1024 byte boundary.
// each channel has source,destination,control,pad (pad word is ignored)
uint32_t ucControlTable[256] __attribute__ ((aligned(1024)));
// channel 30 is at indices 120,121,122 (primary source,destination,control) and
//               at indices 248,249,250 (alternate source,destination,control not used)
#define CH30 (30*4)
#define BIT30 0x40000000
// ************DMA_Init*****************
// Initialize the memory to memory transfer
// This needs to be called once before requesting a transfer
// Inputs:  none
// Outputs: none
void DMA_Init(void){  int i;
  volatile uint32_t delay; 
  for(i=0; i<256; i++){
    ucControlTable[i] = 0;
  }
  SYSCTL_RCGCDMA_R = 0x01;    // µDMA Module Run Mode Clock Gating Control
  delay = SYSCTL_RCGCDMA_R;   // allow time to finish 
  UDMA_CFG_R = 0x01;          // MASTEN Controller Master Enable
  UDMA_CTLBASE_R = (uint32_t)ucControlTable;
  UDMA_PRIOCLR_R = BIT30;     // default, not high priority
  UDMA_ALTCLR_R = BIT30;      // use primary control
  UDMA_USEBURSTCLR_R = BIT30; // responds to both burst and single requests
  UDMA_REQMASKCLR_R = BIT30;  // allow the µDMA controller to recognize requests for this channel
}
// ************DMA_Transfer*****************
// Called to transfer 32-bit words from source to destination
// Inputs:  source is a pointer to the first 32-bit word of the original data
//          destination is a pointer to a place to put the copy
//          count is the number of words to transfer (max is 1024 words)
// Outputs: none
// This routine does not wait for completion
void DMA_Transfer(uint32_t *source, uint32_t *destination, uint32_t count){ 
  ucControlTable[CH30]   = (uint32_t)source+count*4-1;       // last address
  ucControlTable[CH30+1] = (uint32_t)destination+count*4-1;  // last address
  ucControlTable[CH30+2] = 0xAA00C002+((count-1)<<4);             // DMA Channel Control Word (DMACHCTL)
/* DMACHCTL          Bits    Value Description
   DSTINC            31:30   10    32-bit destination address increment
   DSTSIZE           29:28   10    32-bit destination data size
   SRCINC            27:26   10    32-bit source address increment
   SRCSIZE           25:24   10    32-bit source data size
   reserved          23:18   0     Reserved  
   ARBSIZE           17:14   0011  Arbitrates after 8 transfers
   XFERSIZE          13:4  count-1 Transfer count items
   NXTUSEBURST       3       0     N/A for this transfer type
   XFERMODE          2:0     010   Use Auto-request transfer mode
  */
  UDMA_ENASET_R = BIT30;  // µDMA Channel 30 is enabled.
  UDMA_SWREQ_R = BIT30;   // software start, 
  // bit 30 in UDMA_ENASET_R become clear when done
  // bits 2:0 ucControlTable[CH30+2] become clear when done
  // vector 62, NVIC interrupt 46, vector address 0x0000.00F8 could be armed or µDMA Software interrupt
}

// ************DMA_Status*****************
// Can be used to check the status of a previous request
// Inputs:  none
// Outputs: true if still active, false if complete
// This routine does not wait for completion
uint32_t DMA_Status(void){ 
  return (UDMA_ENASET_R&BIT30);  // µDMA Channel 30 enable nit is high if active
}

