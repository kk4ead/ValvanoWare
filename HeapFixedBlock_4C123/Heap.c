// Heap.c
// Runs on any computer
// A fixed-block memory manager.
// Valvano
// April 22, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Program 6.10 through 6.13, Section 6.6

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
#define SIZE 4  // number of 32-bit words in each block
#define NUM 5   // number of blocks
#define NULL 0  // definition of empty pointer
int32_t *FreePt;   // points to the first free block
int32_t Heap[SIZE*NUM];

//------------Heap_Init------------
// Initialize the heap and pointer to the first free block.  Free
// blocks are linked together with a linear linked list
// implemented using the first 32-bit word of each free block to
// point to the next free block.  The last free block points to
// NULL to signify that there are no free blocks after it.
// Input: none
// Output: none
void Heap_Init(void){ int i;
int32_t *pt;
  pt = FreePt = &Heap[0];
  for(i=1; i<NUM; i++){
    *pt = (long)(pt+SIZE);// the first word of a free block points to another free block
    pt = pt + SIZE;       // value of pointer pt should increase by 4*SIZE
  }
  *(long*)pt = NULL;      // the last free block points to NULL
}

//------------Heap_Allocate------------
// Attempt to allocate a block, removing it from the free list.
// The pointer to the first free block is moved to the next free
// block after the block that is being allocated, which is now
// no longer free.
// If no blocks are free, this function returns a NULL pointer.
// Input: none
// Output: pointer to free block or NULL if all blocks full
int32_t *Heap_Allocate(void){
int32_t *pt;
  pt = FreePt;
  if (pt != NULL){        // do nothing if all blocks full
    FreePt = (int32_t*)*pt;  // link next
  }
  return(pt);
}

//------------Heap_Release------------
// Release a block, adding it to the free list, and making it
// available for future use.  The pointer to the first free block
// is moved to this block, and the next free block is the
// previous first free block.  In other words, the newly freed
// block is inserted at the beginning of the free list.
// IMPORTANT: This function does not check anything about the
// pointer and does not check if the block was previously
// allocated.
// An invalid pointer may produce unpredictable behavior.
// Input: pt  pointer to the block to be released
// Output: none
void Heap_Release(int32_t *pt){
int32_t *oldFreePt;
  oldFreePt = FreePt;
  FreePt = pt;            // newly freed block is first
  *pt = (int32_t)oldFreePt;  // newly freed block points to previous first
}
