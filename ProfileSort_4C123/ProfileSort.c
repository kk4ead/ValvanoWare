// ProfileSort.c
// Runs on LM4F120/TM4C123
// Daniel Valvano
// February 23, 2015

/* This example accompanies the books
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Volume 3, Programs 3.17-3.18

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

// PF2 is an output for debugging
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define NONE 0
#define SWDUMP 1
#define HWPORT 2
#define DEBUGTYPE SWDUMP

#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value
#define PF2   (*((volatile uint32_t *)0x40025010))
#define Debug_Set()   (PF2 = 0x04)
#define Debug_Clear() (PF2 = 0x00)

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}
#if (DEBUGTYPE == SWDUMP)
#define DEBUG_LENGTH 100
uint32_t Debug_time[DEBUG_LENGTH];
uint8_t Debug_place[DEBUG_LENGTH];
uint32_t n;
void Debug_Profile(uint8_t p){
  if(n < DEBUG_LENGTH){
    Debug_time[n] = NVIC_ST_CURRENT_R; // record current time
    Debug_place[n] = p;
    n++;
  }
}
#endif
#if (DEBUGTYPE == HWPORT)
#define PROFILE (*((volatile uint32_t *)0x4000703C))
#endif
//------------sort_insertion------------
// Straight insertion sort from "The Art of Computer Programming",
// Donald E. Knuth, Volume 3 Sorting and Searching, Second Edition,
// 1998, pages 80-82.  The input array is copied, sorted, and
// stored in the output array.  The input array is not modified.
// Input: input   pointer to an array of 32-bit numbers to be sorted
//        output  pointer to an array of 32-bit numbers to store sorted output
//        length  number of elements in input and output arrays
// Output: none
void sort_insertion(uint32_t *input, uint32_t *output, uint32_t length){
  uint32_t temp;
  int i, j;
  output[0] = input[0];
#if (DEBUGTYPE == SWDUMP)
  Debug_Profile(0);
#endif
#if (DEBUGTYPE == HWPORT)
  PROFILE = 1;
#endif
  for(j=1; j<length; j=j+1){
    output[j] = input[j];
#if (DEBUGTYPE == SWDUMP)
    Debug_Profile(1);
#endif
#if (DEBUGTYPE == HWPORT)
    PROFILE = 2;
#endif
    for(i=j; i>0; i=i-1){
      if(output[i] < output[i-1]){
        // an entry is smaller than the one before it
        // swap them, causing it to sink toward the beginning
#if (DEBUGTYPE == SWDUMP)
        Debug_Profile(2);
#endif
#if (DEBUGTYPE == HWPORT)
        PROFILE = 4;
#endif
        temp = output[i-1];
        output[i-1] = output[i];
        output[i] = temp;
      } else{
        // an entry is greater or equal to the one before it
        // it will be greater or equal to all before it
#if (DEBUGTYPE == SWDUMP)
        Debug_Profile(3);
        // NOTE: The Debug_place array may skip 3 and go from 2
        // to 1 in the case where index 'i' counts to 0 and the
        // latest entry sinks to the beginning of the list.  In
        // other words, an entry is smaller than all of the
        // ones before it; a change in the left-to-right
        // minimum.
        // Knuth describes the running time of this algorithm:
        // 9*B + 10*N - 3*A - 9 units
        // where,
        // N is the number of records sorted ('length')
        // A is the number of times 'i' decreases to 0 (number of 2's immediately followed by 1's without a 3 in between)
        // B is the number of moves (number of 2's in Debug_place)
#endif
#if (DEBUGTYPE == HWPORT)
        PROFILE = 8;
#endif
        break;
      }
    }
  }
#if (DEBUGTYPE == HWPORT)
  PROFILE = 0;
#endif
}

uint32_t before,elapsed;
// Numerical keys associated with data from "The Art of Computer Programming", Donald E. Knuth, Volume 3 Sorting and Searching, Second Edition, 1998, page 77.
uint32_t keys[16] = {503, 87, 512, 61, 908, 170, 897, 275, 653, 426, 154, 509, 612, 677, 765, 703};
uint32_t sorted[16];       // sorted array

int main(void){
#if (DEBUGTYPE == SWDUMP)
  volatile int KnuthN, KnuthA, KnuthB, KnuthRunTime, i;
#endif
  SYSCTL_RCGCGPIO_R |= 0x20; // activate Port F
  SysTick_Init();            // initialize SysTick timer, Program 4.7
  GPIO_PORTF_DIR_R |= 0x04;  // make PF2 out (built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;// disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;  // enable digital I/O on PF2
                             // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;    // disable analog functionality on PF

#if (DEBUGTYPE == HWPORT)
  SYSCTL_RCGCGPIO_R |= 0x08; // activate Port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};// allow time to finish activating
  GPIO_PORTD_DIR_R |= 0x0F;  // make PD3-0 out
  GPIO_PORTD_AFSEL_R &= ~0x0F;// disable alt funct on PD3-0
  GPIO_PORTD_DEN_R |= 0x0F;  // enable digital I/O on PD3-0
                             // configure PD3-0 as GPIO
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0xFFFF0000)+0x00000000;
  GPIO_PORTD_AMSEL_R &= ~0x0F;// disable analog functionality on PD3-0
#endif

// ****************insertion sort************************
  before = NVIC_ST_CURRENT_R;
  sort_insertion(keys, sorted, 16); // insertion sort into array 'sorted'
  elapsed = (before - NVIC_ST_CURRENT_R - 7)&0x00FFFFFF;
  // the number 7 depends on the instructions before and after test
  // if you remove the call to sort, elapsed measures 0
#if (DEBUGTYPE == SWDUMP)
  KnuthN = 16;               // number of items to be sorted
  KnuthA = 0;                // number of moves
  KnuthB = 0;                // number of changes in the left-to-right minimum
  i = 1;
  while((i < DEBUG_LENGTH) && (Debug_place[i] != 0)){
    if(Debug_place[i] == 2){
      KnuthB = KnuthB + 1;
      if(Debug_place[i+1] == 1){ // possible array out-of-bounds error if DEBUG_LENGTH is too small
        KnuthA = KnuthA + 1;
      }
    }
    i = i + 1;
  }
  KnuthRunTime = 9*KnuthB + 10*KnuthN - 3*KnuthA - 9; // expect 514 for his data
#endif
// *********************************************************

  while(1){
    Debug_Set();                    // PF2=1
    sort_insertion(keys, sorted, 16);// insertion sort into array 'sorted'
    Debug_Clear();                  // PF2=0
  }
}
