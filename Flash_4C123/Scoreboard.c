// Scoreboard.c
// Runs on LM4F120/TM4C123
// Test the functions provided by FlashProgram.c by implementing
// a simple scoreboard for a video game to save and recall high
// scores using flash memory.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
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
#include "FlashProgram.h"
#include "Scoreboard.h"

static uint32_t scoreblock = 0x00000000;      // address of the first score saved in the scoreboard
static uint32_t* boardptr;                    // pointer into the scoreboard; after processing, points to first empty location
static SBEType RAMScoreboard[SCOREBOARDSIZE]; // top scores stored in RAM in descending order

// Check if address offset is valid for scoreboard operation
// Scoreboard blocks must be 1 KB aligned and within range
static int AddrValid(uint32_t addr){
  // check if address offset works for scoreboard
  // must be 1 KB aligned
  return (((addr % 1024) == 0) && (addr <= 0x0003FFFF));
}

//------------Scoreboard_Init------------
// Initialize the scoreboard in a given block of flash memory.
// This block must be 1-KB aligned and must not contain any
// data other than scores.
// Input: addr 1-KB aligned flash memory address to hold scores
// Output: pointer to an array of Scoreboard elements of length 'SCOREBOARDSIZE' (see Scoreboard.h)
SBEType* Scoreboard_Init(uint32_t addr){
  uint32_t initials, score;
  int i, j;
  // initialize blank scoreboard
  for(i=0; i<SCOREBOARDSIZE; i=i+1){
    RAMScoreboard[i].first = ' ';
    RAMScoreboard[i].middle = ' ';
    RAMScoreboard[i].last = ' ';
    RAMScoreboard[i].score = 0;
  }
  if(AddrValid(addr)){
    scoreblock = addr;
    boardptr = (uint32_t *)scoreblock;
    initials = *boardptr;
    score = *(boardptr + 1);
    // step through the flash block, pulling out all scores
    // in the form:
    // [8-bit first initial]
    // [8-bit middle initial]
    // [8-bit last initial]
    // [0x00]
    // [32-bit score]
    while(((initials&0x000000FF) == 0x00000000) && (boardptr <= (uint32_t *)(scoreblock + 0x3F8))){
      // compare the score with the RAM scoreboard so far
      for(i=0; i<SCOREBOARDSIZE; i=i+1){
        if(score > RAMScoreboard[i].score){
          // found a score better than one in RAM
          // shift all lower scores down
          for(j=(SCOREBOARDSIZE-1); j>i; j=j-1){
            RAMScoreboard[j].first = RAMScoreboard[j-1].first;
            RAMScoreboard[j].middle = RAMScoreboard[j-1].middle;
            RAMScoreboard[j].last = RAMScoreboard[j-1].last;
            RAMScoreboard[j].score = RAMScoreboard[j-1].score;
          }
          // insert the new score in the RAM scoreboard
          RAMScoreboard[i].first = (initials&0xFF000000)>>24;
          RAMScoreboard[i].middle = (initials&0x00FF0000)>>16;
          RAMScoreboard[i].last = (initials&0x0000FF00)>>8;
          RAMScoreboard[i].score = score;
          // do not over-write all lower scores
          break;
        }
      }
      boardptr = boardptr + 2;
      initials = *boardptr;
      score = *(boardptr + 1);
    }
  }
  return RAMScoreboard;
}

//------------Scoreboard_Record------------
// Record a score in the scoreboard, regardless of whether or
// not the numerical score is high enough to be shown at the top.
// Input: first  first initial
//        middle middle initial
//        last   last initial
//        score  numerical score earned in the game
// Output: none
void Scoreboard_Record(char first, char middle, char last, uint32_t score){
  int i, j;
  // compare the score with the RAM scoreboard so far
  for(i=0; i<SCOREBOARDSIZE; i=i+1){
    if(score > RAMScoreboard[i].score){
      // found a score better than one in RAM
      // shift all lower scores down
      for(j=(SCOREBOARDSIZE-1); j>i; j=j-1){
        RAMScoreboard[j].first = RAMScoreboard[j-1].first;
        RAMScoreboard[j].middle = RAMScoreboard[j-1].middle;
        RAMScoreboard[j].last = RAMScoreboard[j-1].last;
        RAMScoreboard[j].score = RAMScoreboard[j-1].score;
      }
      // insert the new score in the RAM scoreboard
      RAMScoreboard[i].first = first;
      RAMScoreboard[i].middle = middle;
      RAMScoreboard[i].last = last;
      RAMScoreboard[i].score = score;
      // do not over-write all lower scores
      break;
    }
  }
  if((boardptr <= (uint32_t *)(scoreblock + 0x3F8))){
    // there is still room in the block to hold more scores
    // append the new score to the end of the scores in flash
    Flash_Write((uint32_t)boardptr, (first<<24)|(middle<<16)|(last<<8));
    Flash_Write((uint32_t)(boardptr + 1), score);
    boardptr = boardptr + 2;
  } else{
    // there is no more room in the block to hold more scores
    // clear the block
    Flash_Erase(scoreblock);
    boardptr = (uint32_t *)scoreblock;
    // store the top 'SCOREBOARDSIZE' scores from the RAM buffer
    for(i=0; i<SCOREBOARDSIZE; i=i+1){
      Flash_Write((uint32_t)boardptr, (RAMScoreboard[i].first<<24)|(RAMScoreboard[i].middle<<16)|(RAMScoreboard[i].last<<8));
      Flash_Write((uint32_t)(boardptr + 1), RAMScoreboard[i].score);
      boardptr = boardptr + 2;
    }
  }
}

//------------Scoreboard_Get------------
// Get the top 'SCOREBOARDSIZE' scores and return them in
// descending order.
// Input: none
// Output: pointer to an array of Scoreboard elements of length 'SCOREBOARDSIZE' (see Scoreboard.h)
SBEType* Scoreboard_Get(void){
  return RAMScoreboard;
}
