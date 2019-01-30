// main.c
// Runs on LM4F120/TM4C123
// Test of floating point speed
// Valvano
// September 11, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

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


// no FPU, compiler optimization O3, time optimization
//        float   double float_microLib double_microLIB
//circle   165     175      379            490
// sqrt    282     258     2022           1968
// cos    1561    1531     3826           3789
// atan   2375    2350     6668           6633

// with FPU, compiler optimization O3, time optimization
//        float   double float_microLib double_microLIB
//circle    22         written in assembly
//circle   110     175      328            490
// sqrt    255     226     2038           1980
// sqrt     35         written in assembly
// cos    1860    1667     4016           3975
// atan   2076    2470     6807           6768
#include <stdint.h>
#include <math.h>
#include "SysTick.h"
float CircleArea(float r);
float CalcSqrt(float in);
typedef float testVar;
testVar Circle(testVar r){
  return r*r*3.14159;
}
uint32_t Sum,Ave0,Ave1,Ave2,Ave3,Ave4,Ave5;
volatile testVar Out;
#define SIZE 20000
//uint32_t TimeData[SIZE];
int main(void){ uint32_t time,i;
testVar input,output;
  SysTick_Init();
  Sum = 0;
  input=0;
  for(i=0;i<SIZE;i++){
    SysTick_Start();
    output = CircleArea(input); 
    time = SysTick_Stop();
//    TimeData[i] = time;
    Sum = Sum+time;
    input=input+7.5;
    Out = output;
  }
  Ave0 = Sum/SIZE;

    Sum = 0;
  input=0;
  for(i=0;i<SIZE;i++){
    SysTick_Start();
    output = Circle(input); // float average 165 cycles, double 175
    time = SysTick_Stop();
//    TimeData[i] = time;
    Sum = Sum+time;
    input=input+7.5;
    Out = output;
  }
  Ave1 = Sum/SIZE;

  Sum = 0;
  input=0;
  for(i=0;i<SIZE;i++){
    SysTick_Start();
    output = sqrt(input); // float average 282 cycles, double 258
    time = SysTick_Stop();
//    TimeData[i] = time;
    Sum = Sum+time;
    input=input+0.01;
    Out = output;
  }
  Ave2 = Sum/SIZE;

  Sum = 0;
  input=0;
  for(i=0;i<SIZE;i++){
    SysTick_Start();
    output = CalcSqrt(input);
    time = SysTick_Stop();
//    TimeData[i] = time;
    Sum = Sum+time;
    input=input+0.01;
    Out = output;
  }
  Ave3 = Sum/SIZE;

  Sum = 0;
  input=0;
  for(i=0;i<SIZE;i++){
    SysTick_Start();
    output = cos(input); // float average 1561 cycles, double 1531
    time = SysTick_Stop();
//    TimeData[i] = time;
    Sum = Sum+time;
    input=input+3.14/SIZE;
    Out = output;
  }
  Ave4 = Sum/SIZE;

  Sum = 0;
  input=0.01;
  for(i=0;i<SIZE;i++){
    SysTick_Start();
    output = atan(input); // float average 2375 cycles, double 2350
    time = SysTick_Stop();
//    TimeData[i] = time;
    Sum = Sum+time;
    input=input+1/SIZE;
    Out = output;
  }
  Ave5 = Sum/SIZE;
  while(1);
}
