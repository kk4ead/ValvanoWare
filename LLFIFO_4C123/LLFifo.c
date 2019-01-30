// LLFifo.c
// Runs on any computer
// linked list FIFO
// Jonathan Valvano
// May 14, 2012

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Section 6.6.2 Programs 6.15, 6.16, 6.17, 6.18

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
#include "heap.h"

#define NULL 0  // definition of empty pointer

 struct Node{
  struct Node *Next;
  int32_t Data;
};

typedef struct Node NodeType;
NodeType *PutPt;   // place to put
NodeType *GetPt;   // place to get

void Fifo_Init(void){
  GetPt = NULL;    // Empty when null
  PutPt = NULL;
  Heap_Init();
}

int Fifo_Put(int32_t theData){
NodeType *pt;
  pt = (NodeType*)Heap_Allocate();
  if(!pt){         // check for NULL pointer if heap full
    return(0);     // full
  }
  pt->Data = theData; // store
  pt->Next = NULL;
  if(PutPt){
    PutPt->Next = pt; // Link
  }
  else{
    GetPt = pt;    // first one
  }
  PutPt = pt;
  return(1);       // successful
}

int Fifo_Get(int32_t *datapt){
NodeType *pt;
  if(!GetPt){      // check for NULL pointer if FIFO empty
    return(0);     // empty
  }
  *datapt = GetPt->Data;
  pt = GetPt;
  GetPt = GetPt->Next;
  if(GetPt==NULL){ // one entry
    PutPt = NULL;
  }
  Heap_Release((int32_t*)pt);
  return(1);       // success
}
