// DMASPI.c
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
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// The control table used by the uDMA controller.  This table must be aligned to a 1024 byte boundary.
// each channel has source,destination,control,pad (pad word is ignored)
uint32_t ucControlTable[256] __attribute__ ((aligned(1024)));
// Timer5A uses uDMA channel 8 encoding 3
// channel 8 is at indices  32, 33, 34 (primary source,destination,control) and
//              at indices 160,161,162 (alternate source,destination,control)
#define CH8 (8*4)
#define CH8ALT (8*4+128)
#define BIT8 0x00000100

// ***************** Timer5A_Init ****************
// Activate Timer5A trigger DMA periodically
// Inputs:  period in 12.5nsec
// Outputs: none
void Timer5A_Init(uint16_t period){ volatile uint32_t Delay;
  SYSCTL_RCGCTIMER_R |= 0x20;      // 0) activate timer5
  Delay = 0;                       // wait for completion
  TIMER5_CTL_R &= ~0x00000001;     // 1) disable timer5A during setup
  TIMER5_CFG_R = 0x00000004;       // 2) configure for 16-bit timer mode
  TIMER5_TAMR_R = 0x00000002;      // 3) configure for periodic mode, default down-count settings
  TIMER5_TAILR_R = period-1;       // 4) reload value
  TIMER5_TAPR_R = 0;               // 5) 12.5ns timer5A
  TIMER5_ICR_R = 0x00000001;       // 6) clear timer5A timeout flag
  TIMER5_IMR_R |= 0x00000001;      // 7) arm timeout interrupt
  NVIC_PRI23_R = (NVIC_PRI23_R&0xFFFFFF00)|0x00000040; // 8) priority 2
// interrupts enabled in the main program after all devices initialized
// vector number 108, interrupt number 92
}

// ************DMA_Init*****************
// Initialize the buffer to port transfer, triggered by timer 5A
// This needs to be called once before requesting a transfer
// The source address increments by 2, destination address is fixed
// Call DMA_Start to begin continuous transfer
// Call DMA_Stop to halt the transfer
// Inputs:  period in 12.5nsec
// Outputs: none
void DMA_Init(uint16_t period){int i;
  volatile uint32_t delay; 
  for(i=0; i<256; i++){
    ucControlTable[i] = 0;
  }
  SYSCTL_RCGCDMA_R = 0x01;    // µDMA Module Run Mode Clock Gating Control
  delay = SYSCTL_RCGCDMA_R;   // allow time to finish 
  UDMA_CFG_R = 0x01;          // MASTEN Controller Master Enable
  UDMA_CTLBASE_R = (uint32_t)ucControlTable;
  UDMA_CHMAP1_R = (UDMA_CHMAP1_R&0xFFFFFFF0)|0x00000003;  // timer5A
  UDMA_PRIOCLR_R = BIT8;     // default, not high priority
  UDMA_ALTCLR_R = BIT8;      // use primary control
  UDMA_USEBURSTCLR_R = BIT8; // responds to both burst and single requests
  UDMA_REQMASKCLR_R = BIT8;  // allow the µDMA controller to recognize requests for this channel
  Timer5A_Init(period);
}
uint16_t *SourcePt;               // last address of the source buffer, incremented by 2
volatile uint32_t *DestinationPt;  // fixed address
uint32_t Count;                    // number of halfwords to transmit
// private function used to reprogram regular channel control structure
void static setRegular(void){
  ucControlTable[CH8]   = (uint32_t)SourcePt;           // first and last address
  ucControlTable[CH8+1] = (uint32_t)DestinationPt;      // last address (when DSTINC is 0x3)
  ucControlTable[CH8+2] = 0xD5000003+((Count-1)<<4);         // DMA Channel Control Word (DMACHCTL)
/* DMACHCTL          Bits    Value Description
   DSTINC            31:30   11    no destination address increment
   DSTSIZE           29:28   01    16-bit destination data size
   SRCINC            27:26   01    16-bit source address increment, +2
   SRCSIZE           25:24   01    16-bit source data size
   reserved          23:18   0     Reserved  
   ARBSIZE           17:14   0     Arbitrates after 1 transfer
   XFERSIZE          13:4  count-1 Transfer count items
   NXTUSEBURST       3       0     N/A for this transfer type
   XFERMODE          2:0     011   Use ping-pong transfer mode
  */
}
// private function used to reprogram alternate channel control structure
void static setAlternate(void){
  ucControlTable[CH8ALT]   = (uint32_t)SourcePt;               // first and last address
  ucControlTable[CH8ALT+1] = (uint32_t)DestinationPt;          // last address (when DSTINC is 0x3)
  ucControlTable[CH8ALT+2] = 0xD5000003+((Count-1)<<4);             // DMA Channel Control Word (DMACHCTL)
/* DMACHCTL          Bits    Value Description
   DSTINC            31:30   11    no destination address increment
   DSTSIZE           29:28   01    16-bit destination data size
   SRCINC            27:26   01    16-bit source address increment, +2
   SRCSIZE           25:24   01    16-bit source data size
   reserved          23:18   0     Reserved  
   ARBSIZE           17:14   0     Arbitrates after 1 transfer
   XFERSIZE          13:4  count-1 Transfer count items
   NXTUSEBURST       3       0     N/A for this transfer type
   XFERMODE          2:0     011   Use ping-pong transfer mode
  */
}
// ************DMA_Start*****************
// Called to transfer halfwords from source to destination
// The source address is incremented by two each 16-bit transfer, destination address is fixed
// Inputs:  source is a pointer to a RAM buffer containing waveform to output
//          destination is a pointer to 32-bit DAC device (SSI0_DR_R), least significant 16 bits
//          count is the number of halfwords to transfer (max is 1024 halfwords)
// Outputs: none
// This routine does not wait for completion, runs continuously
void DMA_Start(uint16_t *source, volatile uint32_t *destination, uint32_t count){
  SourcePt = source+count-1;  // last address of source buffer
  DestinationPt = destination;
  Count = count;  // number of halfwords to transmit
  setRegular();  
  setAlternate();  
  NVIC_EN2_R = 0x10000000;         // 9) enable interrupt 92 in NVIC
  // vector number 108, interrupt number 92
  TIMER5_CTL_R |= 0x00000001;      // 10) enable timer5A
  UDMA_ENASET_R |= BIT8;  // µDMA Channel 8 is enabled
  // bits 2:0 ucControlTable[CH8+2] become clear when regular structure done
  // bits 2:0 ucControlTable[CH8ALT+2] become clear when alternate structure done
}

uint32_t NumberOfBuffersSent=0; 
// ************DMA_Status*****************
// Can be used to check the status of the continuous DMA transfer
// Inputs:  none
// Outputs: the number of buffers transferred
uint32_t DMA_Status(void){ 
  return NumberOfBuffersSent;
}

void Timer5A_Handler(void){ // interrupts after each block is transferred
  TIMER5_ICR_R = TIMER_ICR_TATOCINT; // acknowledge timer5A timeout
  NumberOfBuffersSent++;
  if((ucControlTable[CH8+2]&0x0007)==0){     // regular buffer complete
    setRegular();                            // rebuild channel control structure
  }
  if((ucControlTable[CH8ALT+2]&0x0007)==0){  // Alternate buffer complete
    setAlternate();                          // rebuild channel control structure
  }
}

// ************DMA_Stop*****************
// Stop the transfer halfwords from source to destination
// Inputs:  none
// Outputs: none
void DMA_Stop(void){
  UDMA_ENACLR_R = BIT8;  // µDMA Channel 8 is disabled
  NVIC_DIS2_R = 0x10000000;         // 9) disable interrupt 92 in NVIC
  TIMER5_CTL_R &= ~0x00000001;      // 10) disable timer5A
}
