// HeapTestMain.c
// Runs on LM4F120/TM4C123
// Test for memory heap for dynamic memory allocation.

// Jacob Egner 2008-07-31
// modified April 18, 2013 Jonathan Valvano for style
// modified August 10, 2014 for C99 syntax

/* This example accompanies the book
   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015

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

// global so easier to see with the debugger
// Proper style would be to make these variables local to main.
int16_t status; 
int16_t* ptr;
int16_t* p1;
int16_t* p2;
int16_t* p3;
uint8_t* q1;
uint8_t* q2;
uint8_t* q3;
uint8_t* q4;
uint8_t* q5;
uint8_t* q6;
int16_t maxBlockSize;
uint8_t* bigBlock;
heap_stats_t stats;

int main(void){
  int16_t i;

  status = Heap_Init();

  ptr = Heap_Malloc(sizeof(int16_t));
  *ptr = 0x1111;
  status = Heap_Test();

  status = Heap_Free(ptr);
  status = Heap_Test();

  ptr = Heap_Malloc(1);
  status = Heap_Test();

  status = Heap_Free(ptr);
  status = Heap_Test();

  p1 = (int16_t*) Heap_Malloc(1 * sizeof(int16_t));
  p2 = (int16_t*) Heap_Malloc(2 * sizeof(int16_t));
  p3 = (int16_t*) Heap_Malloc(3 * sizeof(int16_t));
  p1[0] = 0xAAAA;
  p2[0] = 0xBBBB;
  p2[1] = 0xBBBB;
  p3[0] = 0xCCCC;
  p3[1] = 0xCCCC;
  p3[2] = 0xCCCC;
  status = Heap_Test();
  stats = Heap_Stats();

  status = Heap_Free(p1);
  status = Heap_Free(p3);
  status = Heap_Test();

  status = Heap_Free(p2);
  status = Heap_Test();
  stats = Heap_Stats();

  for(i = 0; i <= HEAP_SIZE_WORDS; i++){
    ptr = Heap_Malloc(sizeof(int16_t));
  }
  status = Heap_Test();
  stats = Heap_Stats();

  status = Heap_Init();
  q1 = Heap_Malloc(1);
  q2 = Heap_Malloc(2);
  q3 = Heap_Malloc(3);
  q4 = Heap_Malloc(4);
  q5 = Heap_Malloc(5);
  status = Heap_Test();

  *q1 = 0xDD;
  q6 = Heap_Realloc(q1, 6);
  status = Heap_Test();
  stats = Heap_Stats();

  //q1 should point to freed space
  status = Heap_Free(q1);
  status = Heap_Test();

  for(i = 0; i < 6; i++){
    q6[i] = 0xEE;
  }
  q1 = Heap_Realloc(q6, 2);
  status = Heap_Test();

  status = Heap_Init();
  maxBlockSize = HEAP_SIZE_BYTES - 2 * sizeof(int16_t);
  bigBlock = Heap_Malloc(maxBlockSize);
  for(i = 0; i < maxBlockSize; i++){
    bigBlock[i] = 0xFF;
  }
  status = Heap_Test();
  stats = Heap_Stats();
  status = Heap_Free(bigBlock);
  bigBlock = Heap_Calloc(maxBlockSize);
  status = Heap_Test();
  stats = Heap_Stats();
  for(;;){
  } 
}
