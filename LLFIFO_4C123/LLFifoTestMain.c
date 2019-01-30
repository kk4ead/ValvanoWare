// LLFifoTestMain.c
// Runs on any computer
// Test linked list FIFO
// Jonathan Valvano
// May 14, 2012

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Section 6.6.1 Programs 6.10, 6.11, 6.12, 6.13
   Section 6.6.2 Programs 6.14, 6.15, 6.16, 6.17

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
#include "LLFifo.h"

int32_t d1,d2,d3,d4,d5;
uint32_t RunNumber;
void crash(void){
}                                 // put breakpoint here; step one more line; the line above the highlighted line caused the error
int main(void){
  Fifo_Init();                    // initialize linked list FIFO
  RunNumber = 0;
  while(1){                       //        First          Last  Description
    if(Fifo_Put(1)==0) crash();   // FIFO:  [1] [ ] [ ] [ ] [ ]  1 added to FIFO
    if(Fifo_Put(2)==0) crash();   // FIFO:  [1] [2] [ ] [ ] [ ]  2 added to FIFO
    if(Fifo_Get(&d1)==0) crash(); // FIFO:  [2] [ ] [ ] [ ] [ ]  1 removed from FIFO
    if(d1 != 1) crash();
    if(Fifo_Put(3)==0) crash();   // FIFO:  [2] [3] [ ] [ ] [ ]  3 added to FIFO
    if(Fifo_Get(&d2)==0) crash(); // FIFO:  [3] [ ] [ ] [ ] [ ]  2 removed from FIFO
    if(d2 != 2) crash();
    if(Fifo_Put(4)==0) crash();   // FIFO:  [3] [4] [ ] [ ] [ ]  4 added to FIFO
    if(Fifo_Get(&d3)==0) crash(); // FIFO:  [4] [ ] [ ] [ ] [ ]  3 removed from FIFO
    if(d3 != 3) crash();
    if(Fifo_Get(&d4)==0) crash(); // FIFO:  [ ] [ ] [ ] [ ] [ ]  4 removed from FIFO
    if(d4 != 4) crash();
    if(Fifo_Put(5)==0) crash();   // FIFO:  [5] [ ] [ ] [ ] [ ]  5 added to FIFO
    if(Fifo_Get(&d5)==0) crash(); // FIFO:  [ ] [ ] [ ] [ ] [ ]  5 removed from FIFO
    if(d5 != 5) crash();
    if(Fifo_Put(1)==0) crash();   // FIFO:  [1] [ ] [ ] [ ] [ ]  1 added to FIFO
    if(Fifo_Put(2)==0) crash();   // FIFO:  [1] [2] [ ] [ ] [ ]  2 added to FIFO
    if(Fifo_Put(3)==0) crash();   // FIFO:  [1] [2] [3] [ ] [ ]  3 added to FIFO
    if(Fifo_Put(4)==0) crash();   // FIFO:  [1] [2] [3] [4] [ ]  4 added to FIFO
    if(Fifo_Put(5)==0) crash();   // FIFO:  [1] [2] [3] [4] [5]  5 added to FIFO
    if(Fifo_Put(6)==1) crash();   // FIFO:  [1] [2] [3] [4] [5]  expect error; FIFO full
    if(Fifo_Get(&d1)==0) crash(); // FIFO:  [2] [3] [4] [5] [ ]  1 removed from FIFO
    if(d1 != 1) crash();
    if(Fifo_Get(&d2)==0) crash(); // FIFO:  [3] [4] [5] [ ] [ ]  2 removed from FIFO
    if(d2 != 2) crash();
    if(Fifo_Get(&d3)==0) crash(); // FIFO:  [4] [5] [ ] [ ] [ ]  3 removed from FIFO
    if(d3 != 3) crash();
    if(Fifo_Get(&d4)==0) crash(); // FIFO:  [5] [ ] [ ] [ ] [ ]  4 removed from FIFO
    if(d4 != 4) crash();
    if(Fifo_Get(&d5)==0) crash(); // FIFO:  [ ] [ ] [ ] [ ] [ ]  5 removed from FIFO
    if(d5 != 5) crash();
    if(Fifo_Get(&d1)==1) crash(); // FIFO:  [ ] [ ] [ ] [ ] [ ]  expect error; FIFO empty
    RunNumber++;
  }
}
