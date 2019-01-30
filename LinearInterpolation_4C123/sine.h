// Sine.h
// Runs on LM4F120/TM4C123
// Use linear interpolation to implement a sine table lookup using a table
// of only 21 elements.  For input values between table entries, it is
// assumed that the sine function is linear between the entries.  The input
// is given in 8-bit unsigned fixed point with a resolution of 2*pi/256,
// and the output is given in 8-bit signed fixed point with a resolution of
// 1/127.
// In other words:
// sin(90 deg) = sin(pi/2 rad) = Sin(64 inputUnits) = 127 outputUnits = 1
// Daniel Valvano
// September 11, 2013

//  This example accompanies the book
//  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
//  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
//  Program 6.22
//
//Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
//   You may use, edit, run or distribute this file
//   as long as the above copyright notice remains
//THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
//OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
//MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
//VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
//OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
//For more information about my classes, my research, and my books, see
//http://users.ece.utexas.edu/~valvano/

//------------Sin------------
// Calculate the sine value of a given angle using table lookup and
// linear interpolation.
// Input: Ix  8-bit unsigned angle 0 to 255 (units of pi/128)
// Output: Iy 8-bit signed result -127 to +127 (units of 1/127)
int32_t Sin(int32_t Ix);
