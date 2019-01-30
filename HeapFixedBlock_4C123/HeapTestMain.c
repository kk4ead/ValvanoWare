// HeapTestMain.c
// Runs on any computer
// Test the heap functions by continually allocating and releasing
// memory to ensure that the memory manager never crashes.
// Valvano
// May 3, 2015

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
#include "heap.h"

int32_t *Pt1;
int32_t *Pt2;
int32_t *Pt3;
int32_t *Pt4;
int32_t *Pt5;
uint32_t count = 0;

int main(void){
  Heap_Init();
  Pt1 = Heap_Allocate();
  for(;;){
    Pt2 = Heap_Allocate();
    Pt3 = Heap_Allocate();
    Pt4 = Heap_Allocate();
    Pt5 = Heap_Allocate();
    Heap_Release(Pt4);
    Heap_Release(Pt3);
    Heap_Release(Pt2);
    Heap_Release(Pt5);
    count = count + 1;
  }
}
