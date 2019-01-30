// LinearInterpolation.c
// Runs on LM4F120/TM4C123
// Test the Sine() function by testing numbers from 0 to 255 and comparing
// the result of the linear interpolation function to pre-calculated sine
// values.
// Daniel Valvano
// September 11, 2013

//  This example accompanies the book
//  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
//  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
//  Program 6.22
//
//Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
//   You may use, edit, run or distribute this file
//   as long as the above copyright notice remains
//THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
//OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
//MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
//VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
//OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
//For more information about my classes, my research, and my books, see
//http://users.ece.utexas.edu/~valvano/
#include <stdint.h>
#include "sine.h"

const int16_t Expected[256] = {
  0,3,6,9,12,16,19,22,25,28,31,34,37,40,
  43,46,49,51,54,57,60,63,65,68,71,73,76,78,
  81,83,85,88,90,92,94,96,98,100,102,104,106,107,
  109,111,112,113,115,116,117,118,120,121,122,122,123,124,
  125,125,126,126,126,127,127,127,127,127,127,127,126,126,
  126,125,125,124,123,122,122,121,120,118,117,116,115,113,
  112,111,109,107,106,104,102,100,98,96,94,92,90,88,
  85,83,81,78,76,73,71,68,65,63,60,57,54,51,
  49,46,43,40,37,34,31,28,25,22,19,16,12,9,
  6,3,0,-3,-6,-9,-12,-16,-19,-22,-25,-28,-31,-34,
  -37,-40,-43,-46,-49,-51,-54,-57,-60,-63,-65,-68,-71,-73,
  -76,-78,-81,-83,-85,-88,-90,-92,-94,-96,-98,-100,-102,-104,
  -106,-107,-109,-111,-112,-113,-115,-116,-117,-118,-120,-121,-122,-122,
  -123,-124,-125,-125,-126,-126,-126,-127,-127,-127,-127,-127,-127,-127,
  -126,-126,-126,-125,-125,-124,-123,-122,-122,-121,-120,-118,-117,-116,
  -115,-113,-112,-111,-109,-107,-106,-104,-102,-100,-98,-96,-94,-92,
  -90,-88,-85,-83,-81,-78,-76,-73,-71,-68,-65,-63,-60,-57,
  -54,-51,-49,-46,-43,-40,-37,-34,-31,-28,-25,-22,-19,-16,
  -12,-9,-6,-3};
int32_t i;              // global for easy visibility in debugger
int32_t result;         // global for easy visibility in debugger
int16_t Correct = 0;   // (0 <= error < 1) or (-1 < error <= 0)
int16_t OffByOne = 0;  // (1 <= error < 2) or (-2 < error <= -1)
int16_t OffByTwo = 0;  // (2 <= error < 3) or (-3 < error <= -2)
int16_t OffByThree = 0;// (3 <= error < 4) or (-4 < error <= -3)
int16_t OffByFour = 0; // (4 <= error < 5) or (-5 < error <= -4)
int16_t OffByFiveOrMore = 0;// (error >= 5) or (error <= -5)

int main(void){
  for(i=0; i<256; i=i+1){
    result = Sin(i);
    if(result == Expected[i]){
      Correct = Correct + 1;
    } else if(((result - Expected[i]) == 1) || ((result - Expected[i]) == -1)){
      OffByOne = OffByOne + 1;
    } else if(((result - Expected[i]) == 2) || ((result - Expected[i]) == -2)){
      OffByTwo = OffByTwo + 1;
    } else if(((result - Expected[i]) == 3) || ((result - Expected[i]) == -3)){
      OffByThree = OffByThree + 1;
    } else if(((result - Expected[i]) == 4) || ((result - Expected[i]) == -4)){
      OffByFour = OffByFour + 1;
    } else{
      OffByFiveOrMore = OffByFiveOrMore + 1;
    }
  }
  while(1){};
}
