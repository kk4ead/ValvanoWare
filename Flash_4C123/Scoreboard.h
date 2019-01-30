// Scoreboard.h
// Runs on LM4F120/TM4C123
// Test the functions provided by FlashProgram.c by implementing
// a simple scoreboard for a video game to save and recall high
// scores using flash memory.
// Daniel Valvano
// October 21, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2014

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

#define SCOREBOARDSIZE          5           // Number of initials and scores returned

struct ScoreboardElement{                   // An entry in the scoreboard containing the following data
  char first, middle, last;                 // Player's initials
  uint32_t score;                           // Player's numerical score
};
typedef struct ScoreboardElement SBEType;

//------------Scoreboard_Init------------
// Initialize the scoreboard in a given block of flash memory.
// This block must be 1-KB aligned and must not contain any
// data other than scores.
// Input: addr 1-KB aligned flash memory address to hold scores
// Output: pointer to an array of Scoreboard elements of length 'SCOREBOARDSIZE' (see Scoreboard.h)
SBEType* Scoreboard_Init(uint32_t addr);

//------------Scoreboard_Record------------
// Record a score in the scoreboard, regardless of whether or
// not the numerical score is high enough to be shown at the top.
// Input: first  first initial
//        middle middle initial
//        last   last initial
//        score  numerical score earned in the game
// Output: none
void Scoreboard_Record(char first, char middle, char last, uint32_t score);

//------------Scoreboard_Get------------
// Get the top 'SCOREBOARDSIZE' scores and return them in
// descending order.
// Input: none
// Output: pointer to an array of Scoreboard elements of length 'SCOREBOARDSIZE' (see Scoreboard.h)
SBEType* Scoreboard_Get(void);
