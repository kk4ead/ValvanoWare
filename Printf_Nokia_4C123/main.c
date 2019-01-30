// main.c
// Runs on LM4F120/TM4C123
// Nokia5110 LCD 
// Daniel Valvano
// May 6, 2015

/* This example accompanies the books
  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
 
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

#include <stdio.h>
#include <stdint.h> // C99 variable types
#include "Nokia5110.h"
void BookExamples(void){ // examples from the book
  int8_t cc = 0x56; // (‘V’)
  int32_t xx = 100;
  int16_t yy = -100;
  float zz = 3.14159265;
  printf("Hello world\n");	    //Hello world
  printf("cc = %c %d\n",cc,cc);	//cc = V 86 
  printf("cc = %#x\n",cc);	    //cc = 0x56
  printf("xx = %c %d\n",xx,xx);	//xx = d 100 
  printf("xx = %#x\n",xx);	    //xx = 0x64
  printf("yy = %d\n",yy);	    //yy = -100
  printf("%#x   \n",yy);	    //yy = 0xffffff9c
  printf("%e \n",zz);	        //zz = 3.14159e+00
  printf("%E \n",zz);	        //zz = 3.14159E+00
  printf("%f     \n",zz);	    //zz = 3.14159
  printf("%g     \n",zz);	    //zz = 3.14159 (shorter of two, either f or e)
  printf("%3.2f     \n",zz); //zz =  3.14

}
int main(void){ int32_t i,n;
  Output_Init();              // initialize output device
  BookExamples();
  n = 0;
  while(1){
    printf("\ni="); 
    for(i=0; i<1; i++){
      printf("%d ",i+n);
    }
    n = n+1000000; // notice what happend when this goes above 2,147,483,647
  }
}
