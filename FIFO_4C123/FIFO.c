// FIFO.c
// Runs on any Cortex microcontroller
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size.  The file includes a transmit FIFO
// using index implementation and a receive FIFO using pointer
// implementation.  Other index or pointer implementation FIFOs can be
// created using the macros supplied at the end of the file.
// Daniel Valvano
// May 2, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Programs 3.7, 3.8., 3.9 and 3.10 in Section 3.7

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
#include "FIFO.h"


// Two-index implementation of the transmit FIFO
// can hold 0 to TXFIFOSIZE elements
uint32_t volatile TxPutI;// put next
uint32_t volatile TxGetI;// get next
txDataType static TxFifo[TXFIFOSIZE];

// initialize index FIFO
void TxFifo_Init(void){ long sr;
  sr = StartCritical(); // make atomic
  TxPutI = TxGetI = 0;  // Empty
  EndCritical(sr);
}
// add element to end of index FIFO
// return TXFIFOSUCCESS if successful
int TxFifo_Put(txDataType data){
  if((TxPutI-TxGetI) & ~(TXFIFOSIZE-1)){
    return(TXFIFOFAIL); // Failed, fifo full
  }
  TxFifo[TxPutI&(TXFIFOSIZE-1)] = data; // put
  TxPutI++;  // Success, update
  return(TXFIFOSUCCESS);
}
// remove element from front of index FIFO
// return TXFIFOSUCCESS if successful
int TxFifo_Get(txDataType *datapt){
  if(TxPutI == TxGetI ){
    return(TXFIFOFAIL); // Empty if TxPutI=TxGetI
  }
  *datapt = TxFifo[TxGetI&(TXFIFOSIZE-1)];
  TxGetI++;  // Success, update
  return(TXFIFOSUCCESS);
}
// number of elements in index FIFO
// 0 to TXFIFOSIZE-1
uint32_t TxFifo_Size(void){
 return ((uint32_t)(TxPutI-TxGetI));
}

// Two-pointer implementation of the receive FIFO
// can hold 0 to RXFIFOSIZE-1 elements

rxDataType volatile *RxPutPt; // put next
rxDataType volatile *RxGetPt; // get next
rxDataType static RxFifo[RXFIFOSIZE];

// initialize pointer FIFO
void RxFifo_Init(void){ long sr;
  sr = StartCritical();      // make atomic
  RxPutPt = RxGetPt = &RxFifo[0]; // Empty
  EndCritical(sr);
}
// add element to end of pointer FIFO
// return RXFIFOSUCCESS if successful
int RxFifo_Put(rxDataType data){
  rxDataType volatile *nextPutPt;
  nextPutPt = RxPutPt+1;
  if(nextPutPt == &RxFifo[RXFIFOSIZE]){
    nextPutPt = &RxFifo[0];  // wrap
  }
  if(nextPutPt == RxGetPt){
    return(RXFIFOFAIL);      // Failed, fifo full
  }
  else{
    *(RxPutPt) = data;       // Put
    RxPutPt = nextPutPt;     // Success, update
    return(RXFIFOSUCCESS);
  }
}
// remove element from front of pointer FIFO
// return RXFIFOSUCCESS if successful
int RxFifo_Get(rxDataType *datapt){
  if(RxPutPt == RxGetPt ){
    return(RXFIFOFAIL);      // Empty if PutPt=GetPt
  }
  *datapt = *(RxGetPt++);
  if(RxGetPt == &RxFifo[RXFIFOSIZE]){
     RxGetPt = &RxFifo[0];   // wrap
  }
  return(RXFIFOSUCCESS);
}
// number of elements in pointer FIFO
// 0 to RXFIFOSIZE-1
uint32_t RxFifo_Size(void){
  if(RxPutPt < RxGetPt){
    return ((uint32_t)(RxPutPt-RxGetPt+(RXFIFOSIZE*sizeof(rxDataType)))/sizeof(rxDataType));
  }
  return ((uint32_t)(RxPutPt-RxGetPt)/sizeof(rxDataType));
}
