//*****************LCDG.c***************************************
// implementation of the driver for the AGM1264F MODULE
// Jonathan W. Valvano 11/8/11
// modified to run on LM4F120 launchpad on 10/8/12
// compiled on TM4C123 on 9/11/13
// May 2, 2015
/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
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

// Hardware LCD1030 from BG Micro (not tested):
// gnd    =  1- AGM1264F ground
// +5V    =  2- AGM1264F Vcc (with 0.1uF cap to ground)
// pot    =  3- AGM1264F Vo  (center pin of 10k pot)
// PE0    =  4- AGM1264F D/I (0 for command, 1 for data)
// gnd    =  5- AGM1264F R/W (blind cycle synchronization)
// PE1    =  6- AGM1264F E   (1 to latch in data/command)
// PB0    =  7- AGM1264F DB0
// PB1    =  8- AGM1264F DB1
// PB2    =  9- AGM1264F DB2
// PB3    = 10- AGM1264F DB3
// PB4    = 11- AGM1264F DB4
// PB5    = 12- AGM1264F DB5
// PB6    = 13- AGM1264F DB6
// PB7    = 14- AGM1264F DB7
// PE2    = 15- AGM1264F CS1 (controls left half of LCD)
// PE3    = 16- AGM1264F CS2 (controls right half of LCD)
// +5V    = 17- AGM1264F RES (reset)
// pot    = 18- ADM1264F Vee (-10V)
// NC     = 19- A
// NC     = 20- K
// 10k pot from pin 18 to ground, with center to pin 3
// references   http://www.azdisplays.com/prod/g1264f.php
// sample code  http://www.azdisplays.com/PDF/agm1264f_code.pdf
// data sheet   http://www.azdisplays.com/PDF/agm1264f.pdf

// Sparkfun LCD-00710      GDM12864H.pdf 
// HardwareADM12864H (tested):
// +5V    =  1- Vdd
// gnd    =  2- Vss
// pot    =  3- Vo
// PB0    =  4- DB0
// PB1    =  5- DB1
// PB2    =  6- DB2
// PB3    =  7- DB3
// PB4    =  8- DB4
// PB5    =  9- DB5
// PB6    = 10- DB6
// PB7    = 11- DB7
// PE2    = 12- CS2	 (control left side of LCD)
// PE3    = 13- CS1	 (control right side of LCD)
// +5V    = 14- /RES (reset)
// gnd    = 15- R/W (blind cycle synchronization)
// PE0    = 16- D/I (0 for command, 1 for data)
// PE1    = 17- E
// pot    = 18- Vee
// NC     = 19- A
// NC     = 20- K
// 10k pot from pin 18 to ground, with center to pin 3
// http://www.sparkfun.com/datasheets/LCD/GDM12864H.pdf

// BUG NOTICE 11/11/09 -Valvano
// When changing from right to left or from left to right
//   the first write with data=0 goes to two places
// One can reduce the effect of this bug by
// 1) Changing sides less often
// 2) Ignore autoincrement, and set column and page address each time
// 3) Blanking the screen then write 1's to the screen
// GoTo bug fixed on 11/20/09

//******************************************************************

#include "LCDG.h"
#include "../inc/tm4c123gh6pm.h"
#include "SysTick.h"

#define LCDDATA (*((volatile unsigned long *)0x400053FC))   // PORTB
#define LCDCMD (*((volatile unsigned long *)0x4002403C))    // PE3-PE0
#define DI  (*((volatile unsigned long *)0x40024004))       // PE0
#define COMMAND 0             // "DI = COMMAND;" for LCD command
#define DATA    1             // "DI = DATA;" for LCD data
#define E   (*((volatile unsigned long *)0x40024008))       // PE1
#define LATCHLOW  0           // "E = LATCHLOW;" for E pin low
#define LATCHHIGH 2           // "E = LATCHHIGH;" for E pin high
#define CS1 (*((volatile unsigned long *)0x40024010))       // PE2
#define CS2 (*((volatile unsigned long *)0x40024020))       // PE3
#define CS  (*((volatile unsigned long *)0x40024030))       // PE3-PE2
#define NONE  0               // "CS = NONE;" for neither side
#define LEFT  4               // "CS = LEFT;" for left side
#define RIGHT 8               // "CS = RIGHT;" for right side
#define BOTH  12              // "CS = BOTH;" for both sides

#define BusFreq 50            // assuming a 50 MHz bus clock
#define T1usec BusFreq        // 1us
#define T4usec 4*BusFreq      // 4us
#define T6usec 6*BusFreq      // 6us
#define T40usec 40*BusFreq    // 40us

// if 1, includes LCD test examples
// use 0 for more space efficient code
#define TEST 0

// digits are drawn at the top of the byte
const unsigned char TinyFont[10*3]={  // 3 wide by 5 tall
  14,17,14,   // 0
  18,31,16,   // 1
  25,21,23,   // 2
  17,21,31,   // 3
  7,4,31,     // 4
  23,21,9,    // 5
  31,21,28,   // 6
  1,1,31,     // 7
  31,21,31,   // 8
  7,21,31     // 9
};
// digits are drawn one pixel from the top
const unsigned char TinyFont1[10*3]={  // 3 wide by 5 tall
  2*14,2*17,2*14,   // 0
  2*18,2*31,2*16,   // 1
  2*25,2*21,2*23,   // 2
  2*17,2*21,2*31,   // 3
  2*7,2*4,2*31,     // 4
  2*23,2*21,2*9,    // 5
  2*31,2*21,2*28,   // 6
  2*1,2*1,2*31,     // 7
  2*31,2*21,2*31,   // 8
  2*7,2*21,2*31     // 9
};
// two pixels from the top or one pixel from bottom
const unsigned char TinyFont2[10*3]={  // 3 wide by 5 tall
  4*14,4*17,4*14,   // 0
  4*18,4*31,4*16,   // 1
  4*25,4*21,4*23,   // 2
  4*17,4*21,4*31,   // 3
  4*7,4*4,4*31,     // 4
  4*23,4*21,4*9,    // 5
  4*31,4*21,4*28,   // 6
  4*1,4*1,4*31,     // 7
  4*31,4*21,4*31,   // 8
  4*7,4*21,4*31     // 9
};
// three pixels from the top or at the bottom
const unsigned char TinyFont3[10*3]={  // 3 wide by 5 tall
  8*14,8*17,8*14,   // 0
  8*18,8*31,8*16,   // 1
  8*25,8*21,8*23,   // 2
  8*17,8*21,8*31,   // 3
  8*7,8*4,8*31,     // 4
  8*23,8*21,8*9,    // 5
  8*31,8*21,8*28,   // 6
  8*1,8*1,8*31,     // 7
  8*31,8*21,8*31,   // 8
  8*7,8*21,8*31     // 9
};
// 5 wide by 7 tall font
const unsigned char Font[96*5]={   // no numbers with bit7=1
  0,0,0,0,0,           // 32  space
  0,0,95,0,0,          // 33  !
  0,7,0,7,0,           // 34  "
  20,127,20,127,20,    // 35  #
  36,42,127,42,18,     // 36  $
  35,19,8,100,98,      // 37  %
  54,73,85,34,80,      // 38  &
  0,5,3,0,0,           // 39  quote
  0,28,34,65,0,        // 40  (
  0,65,34,28,0,        // 41  )
  20,8,62,8,20,        // 42  *
  8,8,62,8,8,          // 43  plus
  0,80,48,0,0,         // 44  ,
  8,8,8,8,8,           // 45  minus
  0,112,112,112,0,     // 46  .
  32,16,8,4,2,         // 47  /
  62,81,73,69,62,      // 48  0
  0,66,127,64,0,       // 49  1
  66,97,81,73,70,      // 50  2
  33,65,69,75,49,      // 51  3
  24,20,18,127,16,     // 52  4
  39,69,69,69,57,      // 53  5
  60,74,73,73,48,      // 54  6
  3,1,113,9,7,         // 55  7
  54,73,73,73,54,      // 56  8
  6,73,73,41,30,       // 57  9
  0,54,54,0,0,         // 58  :
  0,86,54,0,0,         // 59  ;
  8,20,34,65,0,        // 60  <
  20,20,20,20,20,      // 61  equals
  65,34,20,8,0,        // 62  >
  2,1,81,9,6,          // 63  ?
  50,73,121,65,62,     // 64  @
  126,17,17,17,126,    // 65  A
  127,73,73,73,54,     // 66  B
  62,65,65,65,34,      // 67  C
  127,65,65,65,62,     // 68  D
  127,73,73,73,65,     // 69  E
  127,9,9,9,1,         // 70  F
  62,65,73,73,122,     // 71  G
  127,8,8,8,127,       // 72  H
  65,65,127,65,65,     // 73  I
  32,64,65,63,1,       // 74  J
  127,8,20,34,65,      // 75  K
  127,64,64,64,64,     // 76  L
  127,2,12,2,127,      // 77  M
  127,6,24,96,127,     // 78  N
  62,65,65,65,62,      // 79  O
  127,9,9,9,6,         // 80  P
  62,65,81,33,94,      // 81  Q
  127,9,25,41,70,      // 82  R
  70,73,73,73,49,      // 83  S
  1,1,127,1,1,         // 84  T
  63,64,64,64,63,      // 85  U
  31,32,64,32,31,      // 86  V
  63,64,56,64,63,      // 87  W
  99,20,8,20,99,       // 88  X
  7,8,112,8,7,         // 89  Y
  97,81,73,69,67,      // 90  Z
  0,127,65,65,0,       // 91  [
  2,4,8,16,32,         // 92  back slash
  0,65,65,127,0,       // 93  ]
  4,2,1,2,4,           // 94  ^
  64,64,64,64,64,      // 95  _
  0,1,2,4,0,           // 96  quote
  32,84,84,84,120,     // 97  a
  127,72,68,68,56,     // 98  b
  56,68,68,68,32,      // 99  c
  56,68,68,72,127,     // 100  d
  56,84,84,84,24,      // 101  e
  8,126,9,1,2,         // 102  f
  8,84,84,84,60,       // 103  g
  127,8,4,4,120,       // 104  h
  0,72,125,64,0,       // 105  i
  32,64,68,61,0,       // 106  j
  127,16,40,68,0,      // 107  k
  0,65,127,64,0,       // 108  l
  124,4,24,4,120,      // 109  m
  124,8,4,4,120,       // 110  n
  56,68,68,68,56,      // 111  o
  124,20,20,20,8,      // 112  p
  12,18,18,20,126,     // 113  q
  124,8,4,4,8,         // 114  r
  72,84,84,84,36,      // 115  s
  4,63,68,64,32,       // 116  t
  60,64,64,32,124,     // 117  u
  28,32,64,32,28,      // 118  v
  60,64,48,64,60,      // 119  w
  68,40,16,40,68,      // 120  x
  12,80,80,80,60,      // 121  y
  68,100,84,76,68,     // 122  z
  0,65,54,8,0,         // 123  }
  0,0,127,0,0,         // 124  |
  0,8,54,65,0,         // 125  {
  8,4,8,16,8,          // 126  ~
  31,36,124,36,31      // 127  UT sign
};



static unsigned short OpenFlag=0;
unsigned char Column1; // column position
unsigned char bLeft1;  // to be placed into CS1, in LCD_OutChar
unsigned char bRight1; // to be placed into CS2, in LCD_OutChar
unsigned char Page;
unsigned char bDown;    // true if want font shifted down
unsigned char TinyDown; // 0 no shift, 1,2,3 shift down
unsigned char Xcolumn;  // byte number 56-7F on left, 40-79 on right
unsigned char bRightx;  // to be placed into CS2
unsigned char bLeftx;   // to be placed into CS1
unsigned char PlotPixel;
// number of data points drawn into the same X-axis pixel
// 4 means it takes 400 calls to LCD_Plot to draw one sweep
unsigned short SubCount; // goes PlotPixel down to 1

unsigned short XaxisResolution;
// X axis numbers are  0 to 999
// resolution>3 means no numbers are displayed
// number->       0    9    99   999
// 0  shown as   0.    9.   99.  999.
// 1  shown as  0.0   0.9   9.9  99.9
// 2  shown as 0.00  0.09  0.99  9.99
// 3  shown as .000  .009  .099  .999

unsigned short YaxisResolution;
// Y axis numbers are  -99 to 99
// any other value means no numbers are displayed
// 0  shown as -99     -1    0   1   99
// 1  shown as -9.9  -0.1  0.0 0.1  9.9
// 2  shown as -.99  -.01  .00 .01  .99
short MinY,RangeY;        // used to scale input data

// ********* lcdCmd***********
// Output command to AGM1264F 128-bit by 64-bit graphics display
// Inputs: 8-bit instruction
// Outputs: none
void lcdCmd(unsigned char instruction){
  // R/W=0, write mode  default, R/W=0 always
  // normally D/I will be left at D/I=1 for data
  LCDDATA = instruction;
  E = LATCHLOW;         // E=0
  DI = COMMAND;         // D/I=0, COMMAND WRITE
  SysTick_Wait(T6usec); // wait 6us
  E = LATCHHIGH;        // E=1, D/I=0, E pulse width > 450ns
  SysTick_Wait(T6usec); // wait 6us
  E = LATCHLOW;         // falling edge latch, setup time 200ns
  DI = DATA;            // D/I=1 default state is data
  SysTick_Wait(T40usec);// wait 40us
}

// ********* lcdData***********
// Output data to AGM1264F 128-bit by 64-bit graphics display
// Inputs: 8-bit data
// Outputs: none
void lcdData(unsigned char data){
  // R/W=0, write mode  default, R/W=0 always
  // normally D/I will be left at D/I=1 for data
  LCDDATA = data;
  E = LATCHLOW;         // E=0
  DI = DATA;            // D/I=1, DATA WRITE
  SysTick_Wait(T6usec); // wait 6us
  E = LATCHHIGH;        // E=1, E pulse width > 450ns
  SysTick_Wait(T6usec); // wait 6us
  E = LATCHLOW;         // falling edge latch, setup time 200ns
  SysTick_Wait(T40usec);// wait 40us
}

// ********* LCD_Init***********
// Initialize AGM1264F 128-bit by 64-bit graphics display
// activates SysTick at 50 MHz, assumes PLL active
// Input: none
// Output: none
// does not clear the display
void LCD_Init(void){ volatile long delay;
  SYSCTL_RCGC2_R |= 0x00000012;  // 1) activate clock for Ports B and E
  delay = SYSCTL_RCGC2_R;        // allow time for clock to start
  GPIO_PORTB_DIR_R = 0xFF;       // 2) set direction register
  GPIO_PORTE_DIR_R |= 0x0F;
  GPIO_PORTB_AFSEL_R = 0x00;     // 3) regular port function
  GPIO_PORTE_AFSEL_R &= ~0x0F;
  GPIO_PORTB_DEN_R = 0xFF;       // 4) enable digital port
  GPIO_PORTE_DEN_R |= 0x0F;
  GPIO_PORTB_DR8R_R = 0xFF;      // 5) enable 8 mA drive
  GPIO_PORTE_DR8R_R |= 0x0F;
  GPIO_PORTB_PCTL_R = 0;         // 6) configure as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFFFF0000)+0x00000000;
  GPIO_PORTB_AMSEL_R = 0;        // 7) disable analog functionality on GPIO pins
  GPIO_PORTE_AMSEL_R = (GPIO_PORTE_AMSEL_R&0xFFFF0000)+0x00000000;
  SysTick_Init();                // Program 2.10
  DI = DATA;                     // default mode is data
  E = LATCHLOW;                  // inactive
  CS = BOTH;                     // talk to both LCD controllers
  SysTick_Wait10ms(10);          // let it warm up
  lcdCmd(0x3F);                  // display=ON
  lcdCmd(0xB8);                  // Page address (0 to 7) is 0
  lcdCmd(0x40);                  // Column address (0 to 63) is 0
  lcdCmd(0xC0);                  // Y=0 is at top
  OpenFlag = 1;                  // device open
  Column1 = 0x41;                // column position
  bLeft1 = LEFT;
  bRight1 = NONE;
  Page = 0xB8;
  bDown = 0;                     // true if want font shifted down
  TinyDown = 0;                  // 0 no shift, 1,2,3 shift down
}

// ********* LCD_Clear***********
// Clear the entire 1024 byte (8192 bit) image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: value to write into all bytes of display RAM
// Output: none
// e.g., LCD_Clear(0);  // makes all pixels off
void LCD_Clear(unsigned char data){
unsigned char page;
  int i;
  if(OpenFlag == 0) return;
  for(page = 0xB8; page< 0xB8+8; page++){ // pages 0 to 7
    CS = RIGHT;      // right enable
    lcdCmd(page);    // Page address (0 to 7)
    lcdCmd(0x40);    // Column = 0
    for(i=64; i>0; i--){
      lcdData(data); // copy one byte to right side
    }
    CS = LEFT;       // left enable
    lcdCmd(page);    // Page address (0 to 7)
    lcdCmd(0x40);    // Column = 0
    for(i=64; i>0; i--){
      lcdData(data); // copy one byte to left side
    }
  }
}

// page   is 0xB8 to 0xBF for pages 0 to 7
// column is 0x40 to 0x7F for columns 0 to 63
void OutByte(unsigned char page, unsigned char column,unsigned char data){
  lcdCmd(page);    // Page address (0 to 7)
  lcdCmd(column);  // Column = 0 to 63
  lcdData(data);   // data
}

// ********* LCD_VTest***********
// Clear the vertical lines image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: none
// Output: none
void LCD_VTest(unsigned char num1, unsigned char num2){
unsigned char page;
  int i;
  CS = LEFT;       // left enable
  for(page=0xB8; page<=0xBF; page++){ // pages 0 to 7
    for(i=0x40; i<=0x7F; i=i+2){
      OutByte(page,i,num1);
      OutByte(page,i+1,num2);
    }
  }
  CS = RIGHT;      // right enable
  for(page=0xB8; page<=0xBF; page++){ // pages 0 to 7
    for(i=0x40; i<=0x7F; i=i+2){
      OutByte(page,i,num1);
      OutByte(page,i+1,num2);
    }
  }
}

// ********* LCD_DrawImage***********
// Draw an entire 1024 byte (8192 bit) image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: pointer to 1024 bytes of data
// Output: none
void LCD_DrawImage(const unsigned char *pt){
unsigned char page;
  int i;
  if(OpenFlag == 0) return;
  LCD_Clear(0);
  CS = LEFT;       // left enable
  for(page=0xB8; page<=0xBF; page++){ // pages 0 to 7
    for(i=0x40; i<=0x7F; i=i+1){
      if(*pt){
        OutByte(page,i,*pt);  // copy one byte to left
      }
      pt++;
    }
  }

  CS = RIGHT;      // right enable
  for(page=0xB8; page<=0xBF; page++){ // pages 0 to 7
    for(i=0x40; i<=0x7F; i=i+1){
      if(*pt){
        OutByte(page,i,*pt);  // copy one byte to right
      }
      pt++;
    }
  }
}
#if TEST
unsigned char const TestImage2[1024]={
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,
  0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00
};
unsigned char const TestImage[1024] ={
// left half
0,0,0,0,0,0,0,0,32,112,112,112,112,112,112,240,240,240,240,224,224,224,224,192,192,192,128,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,3,3,7,7,15,15,31,63,126,252,248,248,240,240,224,192,128,128,0,0,0,0,0,0,0,0,0,0,0,0,0,128,0,0,128,0,0,128,0,0,128,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,3,7,15,31,31,63,63,127,255,254,254,252,252,252,252,252,252,254,255,255,255,255,255,255,255,255,255,255,255,255,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,28,60,62,63,127,127,127,127,63,63,63,31,31,127,255,255,255,255,255,255,255,255,255,255,255,255,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,31,255,255,255,255,255,255,255,255,255,255,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,15,255,255,255,255,255,255,255,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,254,255,255,255,255,255,255,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,7,15,31,31,31,31,
// right half
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,128,192,192,192,224,224,224,224,240,240,240,240,112,112,112,112,112,112,32,0,0,0,0,0,0,0,0,
0,128,0,0,128,0,0,128,0,0,128,0,0,0,0,0,0,0,0,0,0,0,0,0,128,128,192,224,240,240,248,248,252,126,63,31,15,15,7,7,3,3,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,255,255,255,255,255,255,255,255,255,255,255,254,252,252,252,252,252,252,254,254,255,127,63,63,31,31,15,7,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,255,255,255,255,255,255,255,255,255,255,255,127,31,31,63,63,63,127,127,127,127,63,62,60,28,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,255,255,255,255,255,255,255,255,255,31,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,255,255,255,255,255,255,15,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,255,255,255,255,255,254,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
31,31,31,31,15,7,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
// ********* LCD_DrawImageTest***********
// Draw an entire 1024 byte (8192 bit) image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: none, used above test image
// Output: none
void LCD_DrawImageTest(void){
  LCD_DrawImage(TestImage);
}
#endif

// ********* LCD_OutChar***********
// Output ASCII character on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: 7-bit ASCII to display
// Output: none
// letter must be between 32 and 127 inclusive
// execute LCD_GoTo to specify cursor location
void LCD_OutChar(unsigned char letter){
unsigned short i,cnt;
  if(OpenFlag == 0) return;
// page 0 is 0xB8, varies from 0xB7 to 0xBF
  if(letter<32) return;
  if(letter>127) return;
  i = 5*(letter-32); // index into font table
  CS2 = bRight1;   // right enable
  CS1 = bLeft1;    // left enable
  lcdCmd(Page);    // Page address 0 to 7
  lcdCmd(Column1); // Column = 0
  for(cnt=5; cnt>0; cnt--){
    if(bDown){
      lcdData(Font[i]<<1);  // copy one byte, shifted down
    } else{
      lcdData(Font[i]);  // copy one byte
    }
    i++;
    Column1++;
    if(bLeft1&&(Column1==0x80)){
      Column1 = 0x40;
      bLeft1 = NONE;
      bRight1 = RIGHT; // switch to right side
      CS2 = bRight1;   // right enable
      CS1 = bLeft1;    // left enable
      lcdCmd(Page);    // Page address 0 to 7)
      lcdCmd(Column1); // Column = 0
    }
    if(bRight1&&(Column1==0x7F)){
      Column1 = 0x41;
      bLeft1 = LEFT;
      bRight1 = NONE;  // switch to left side
      CS2 = bRight1;   // right enable
      CS1 = bLeft1;    // left enable
      lcdCmd(Page);    // Page address 0 to 7)
      lcdCmd(Column1); // Column = 0
    }
  }
  lcdData(0);  // inter-character space copy one byte
  Column1++;
  if(bLeft1&&(Column1==0x80)){
    Column1 = 0x40;
    bLeft1 = NONE;
    bRight1 = RIGHT; // switch to right side
    CS2 = bRight1;   // right enable
    CS1 = bLeft1;    // left enable
    lcdCmd(Page);    // Page address 0 to 7)
    lcdCmd(Column1); // Column = 0
  }
  if(bRight1&&(Column1==0x7F)){
    Column1 = 0x41;
    bLeft1 = LEFT;
    bRight1 = NONE;  // switch to left side
    CS2 = bRight1;   // right enable
    CS1 = bLeft1;    // left enable
    lcdCmd(Page);    // Page address 0 to 7)
    lcdCmd(Column1); // Column = 0
  }
}

//---------------------LCD_OutString--------------
// Display String
// Input: pointer to NULL-terminationed ASCII string
// Output: none
void LCD_OutString(char *pt){
  if(OpenFlag==0){
    return;  // not open
  }
  while(*pt){
    LCD_OutChar((unsigned char)*pt);
    pt++;
  }
}

//-----------------------LCD_OutDec-----------------------
// Output a 16-bit number in unsigned decimal format
// Input: 16-bit unsigned number
// Output: none
// fixed size 5 digits of output, right justified
void LCD_OutDec(unsigned short n){
  if(OpenFlag==0){
    return;  // not open
  }
  if(n < 10){
    LCD_OutString("    ");
    LCD_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    LCD_OutString("   ");
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
      LCD_OutString("  ");
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      LCD_OutChar(' ');
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else {
      LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
}

//-----------------------LCD_OutSDec-----------------------
// Output a 16-bit number in signed decimal format
// Input: 16-bit signed number
// Output: none
// fixed size 6 digits of output, right justified
void LCD_OutSDec(short n){
unsigned char sign=' ';
  if(OpenFlag==0){
    return;  // not open
  }
  if(n < 0){
    n = -n;       // negative
    sign = '-';
  }
  if(n < 10){
    LCD_OutString("    ");
    LCD_OutChar(sign);
    LCD_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    LCD_OutString("   ");
    LCD_OutChar(sign);
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
      LCD_OutString("  ");
      LCD_OutChar(sign);
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      LCD_OutChar(' ');
      LCD_OutChar(sign);
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else {
      LCD_OutChar(sign);
      LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
}

//-----------------------LCD_OutFix1-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.1
// Input: 16-bit unsigned number
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 1234.5
void LCD_OutFix1(unsigned short n){
  if(OpenFlag==0){
    return;  // not open
  }
  if(n < 10){
    LCD_OutString("   0.");
    LCD_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    LCD_OutString("   ");
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar('.');      /* decimal point */
    LCD_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
      LCD_OutString("  ");
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar('.');      /* decimal point */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      LCD_OutChar(' ');
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar('.');      /* decimal point */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else {
      LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar('.');      /* decimal point */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
}

//-----------------------LCD_OutFix2-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01
// Input: 16-bit unsigned number
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 123.45
void LCD_OutFix2(unsigned short n){
  if(OpenFlag==0){
    return;  // not open
  }
  if(n < 10){
    LCD_OutString("  0.0");
    LCD_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    LCD_OutString("  0.");
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
    LCD_OutString("  ");
    LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    LCD_OutChar('.');      /* decimal point */
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  }
  else if(n<10000){
    LCD_OutChar(' ');
    LCD_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    LCD_OutChar('.');      /* decimal point */
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  }
  else {
    LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
    n = n%10000;
    LCD_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    LCD_OutChar('.');      /* decimal point */
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  }
}

//-----------------------LCD_OutFix2b-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01
// Input: 16-bit unsigned number, 0 to 9999
// Output: none
// fixed size is 5 characters of output, right justified
// if input is 1234, then display is 12.34
void LCD_OutFix2b(unsigned short n){
  if(OpenFlag==0){
    return;  // not open
  }

  if(n < 10){
    LCD_OutString(" 0.0");
    LCD_OutChar(n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    LCD_OutString(" 0.");
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
    LCD_OutChar(' ');
    LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    LCD_OutChar('.');      /* decimal point */
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  }
  else if(n<10000){
    LCD_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    LCD_OutChar('.');       /* decimal point */
    LCD_OutChar(n/10+'0');  /* tens digit */
    LCD_OutChar(n%10+'0');  /* ones digit */
  }
  else {
    LCD_OutString("**.**");
  }
}

//-----------------------LCD_OutFix3-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.001
// Input: 16-bit unsigned number
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 12.345
void LCD_OutFix3(unsigned short n){
  if(OpenFlag==0){
    return;  // not open
  }
  if(n < 10){
    LCD_OutString(" 0.00");
    LCD_OutChar(n+'0');  /* n is between 0 and 9 */
  } else if(n<100){
    LCD_OutString(" 0.0");
    LCD_OutChar(n/10+'0'); /* tens digit */
    LCD_OutChar(n%10+'0'); /* ones digit */
  } else if(n<1000){
      LCD_OutString(" 0.");
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      LCD_OutChar(' ');
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      LCD_OutChar('.');      /* decimal point */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else {
      LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      LCD_OutChar(n/1000+'0'); /* thousands digit */
      LCD_OutChar('.');      /* decimal point */
      n = n%1000;
      LCD_OutChar(n/100+'0'); /* hundreds digit */
      n = n%100;
      LCD_OutChar(n/10+'0'); /* tens digit */
      LCD_OutChar(n%10+'0'); /* ones digit */
    }
}

//-----------------------LCD_GoTo-----------------------
// Move cursor
// Input: line number is 1 to 8, column from 1 to 21
// Output: none
// errors: it will ignore legal addresses
void LCD_GoTo(int line, int column){
  if(OpenFlag==0){
    return;  // not open
  }
  if((line<1) || (line>8)) return;
  if((column<1) || (column>21)) return;
  if(line<5){
    bDown = 0;             // normal position on lines 1,2,3,4
  } else{
    bDown = 0xFF;          // shifted down on lines 5,6,7,8
  }
  Page = 0xB8+line-1;      // 0xB8 to 0xBF
  if(column<12){
    Column1 = 59+6*column; // 0x41+6*(column-1);
    bLeft1 = LEFT;
    bRight1 = NONE;        // on left side
  } else{
    Column1 = 6*column-5;  // 0x43+6*(column-12);
    bLeft1 = NONE;
    bRight1 = RIGHT;       // on right side
  }
}

//---------lcdTinyOutDigit-------------------------
// display tiny digits
// input: digit from 0 to 9
// output: none
// no error checking
void lcdTinyOutDigit(unsigned short digit){
unsigned char *pt;
  if(TinyDown==0){
    pt = (unsigned char *) &TinyFont[digit*3];
  } else if(TinyDown==1){
    pt = (unsigned char *) &TinyFont1[digit*3];
  } else if(TinyDown==2){
    pt = (unsigned char *) &TinyFont2[digit*3];
  } else {
    pt = (unsigned char *) &TinyFont3[digit*3];
  }
  lcdData(*pt++);  // copy three bytes
  lcdData(*pt++);
  lcdData(*pt++);
  lcdData(0);      // intercharacter space
}

//---------lcdTinyOutPoint-------------------------
// display tiny decimal point
// input: none
// output: none
void lcdTinyOutPoint(void){
  if(TinyDown==0){
    lcdData(16);
  } else if(TinyDown==1){
    lcdData(32);
  } else if(TinyDown==2){
    lcdData(64);
  } else {
    lcdData(128);
  }
  lcdData(0);      // intercharacter space
}

//---------lcdTinyOutSpace-------------------------
// display space
// input: none
// output: none
void lcdTinyOutSpace(void){
  lcdData(0);      // intercharacter space
  lcdData(0);      // intercharacter space
  lcdData(0);      // intercharacter space
  lcdData(0);      // intercharacter space
}

//---------lcdTinyOutMinus-------------------------
// display tiny minus sign
// input: none
// output: none
void lcdTinyOutMinus(void){
  if(TinyDown==0){
    lcdData(4);
    lcdData(4);
  } else if(TinyDown==1){
    lcdData(8);
    lcdData(8);
  } else if(TinyDown==2){
    lcdData(16);
    lcdData(16);
  } else {
    lcdData(32);
    lcdData(32);
  }
  lcdData(0);      // intercharacter space
}

//---------lcdTinyOutPlus-------------------------
// display a little space instead of plus sign
// input: none
// output: none
void lcdTinyOutPlus(void){
  lcdData(0);      // intercharacter space
  lcdData(0);      // intercharacter space
  lcdData(0);      // intercharacter space
}

//------lcdTinyOutFix------------------------
// display 4 character tiny number
// input: num 0 to 999
// output:none
// no error checking
void lcdTinyOutFix(unsigned short num){
  if(XaxisResolution == 0){      // shown as   0.    9.   99.  999.
    if(num < 10){
      lcdTinyOutSpace();
      lcdTinyOutSpace();
    } else if(num < 100){
      lcdTinyOutSpace();
      lcdTinyOutDigit(num/10);   // tens digit 1 to 9
    } else{
      lcdTinyOutDigit(num/100);  // hundreds digit 1 to 9
      num = num%100;             // 0 to 99
      lcdTinyOutDigit(num/10);   // tens digit 1 to 9
    }
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
    lcdTinyOutPoint();
  }else if(XaxisResolution == 1){// shown as  0.0   0.9   9.9  99.9
    if(num < 100){
      lcdTinyOutSpace();
    } else{
      lcdTinyOutDigit(num/100);  // hundreds digit 1 to 9
      num = num%100;             // 0 to 99
    }
    lcdTinyOutDigit(num/10);     // tens digit 1 to 9
    lcdTinyOutPoint();
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
  }else if(XaxisResolution == 2){//shown as 0.00  0.09  0.99  9.99
    lcdTinyOutDigit(num/100);    // hundreds digit 1 to 9
    num = num%100;               // 0 to 99
    lcdTinyOutPoint();
    lcdTinyOutDigit(num/10);     // tens digit 1 to 9
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
  }else if(XaxisResolution == 3){// shown as .000  .009  .099  .999
    lcdTinyOutDigit(num/100);    // hundreds digit 1 to 9
    lcdTinyOutPoint();
    num = num%100;               // 0 to 99
    lcdTinyOutDigit(num/10);     // tens digit 1 to 9
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
  }
}

//-----------------------LCD_PlotXaxis-----------------------
// Draw X axis
// Input: min max, resolution, and label
// Output: none
// X axis numbers min and max range from 0 to 999
// resolution>3 means no numbers are displayed
// number->       0    9    99   999
// 0  shown as   0.    9.   99.  999.
// 1  shown as  0.0   0.9   9.9  99.9
// 2  shown as 0.00  0.09  0.99  9.99
// 3  shown as .000  .009  .099  .999
// label has 13 character maximum size
// errors: min and max must be less than or equal to 999
void LCD_PlotXaxis(unsigned short min, unsigned short max,
  unsigned short resolution, char *label){
  unsigned short i;
  char Xlabel[14];          // maximum of 13 characters
  if(OpenFlag==0){
    return;  // not open
  }
  if((min > max)||(max > 999)){
    return;  // bad parameters
  }
  XaxisResolution = resolution;
  i = 0;
  do{
    Xlabel[i] = *label;
    label++; i++;
  }
  while(i<14);
  Xlabel[13] = 0;    // truncate to 13 characters maximum
  LCD_GoTo(7,6);     // second to last row
  LCD_OutString(Xlabel);
  TinyDown = 2;      // shift down two pixels
  CS = LEFT;         // paint left number on left side
  lcdCmd(0xBE);      // Page 7
  lcdCmd(0x4C);      // byte number = 12
  lcdTinyOutFix(min);
  CS = RIGHT;        // paint right number on right side
  lcdCmd(0xBE);      // Page 7
  lcdCmd(0x72);      // byte number = 50
  lcdTinyOutFix(max);
}

//------lcdTinyOutFix2------------------------
// display 4 character tiny number
// input: num -99 to 99
// output:none
// no error checking
void lcdTinyOutFix2(short snum){
unsigned short num;
unsigned short bSign;
  if(snum<0){
    bSign = 1;
    num = -snum;
  } else{
    bSign = 0;
    num = snum;
  }
  if(YaxisResolution == 0){      // shown as  -99     -1    0   1   99
    if(num < 10){
      lcdTinyOutSpace();         // no tens digit
      if(bSign){
        lcdTinyOutMinus();
      } else{
        lcdTinyOutPlus();
      }
    } else{
      if(bSign){
        lcdTinyOutMinus();
      } else{
        lcdTinyOutPlus();
      }
      lcdTinyOutDigit(num/10);   // tens digit 1 to 9
    }
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
    lcdTinyOutPoint();
  }else if(YaxisResolution == 1){// shown as  -9.9  -0.1  0.0 0.1  9.9
    if(bSign){
      lcdTinyOutMinus();
    } else{
      lcdTinyOutPlus();
    }
    lcdTinyOutDigit(num/10);     // tens digit 1 to 9
    lcdTinyOutPoint();
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
  }else if(YaxisResolution == 2){// shown as -.99  -.01  .00 .01  .99
    if(bSign){
      lcdTinyOutMinus();
    } else{
      lcdTinyOutPlus();
    }
    lcdTinyOutPoint();
    lcdTinyOutDigit(num/10);     // tens digit 1 to 9
    lcdTinyOutDigit(num%10);     // ones digit 1 to 9
  }
}

// ********* lcdFillYline***********
// get one line of Ylabel
//    AGM1264F 128-bit by 64-bit graphics display
// Input: ASCII string, 8 characters max
//        mask is 0x01,0x02,0x04,0x08,0x10,0x20 or 0x40
// Output: none
// characters must be between 32 and 126 inclusive
unsigned char Yline[6];  // data for side-ways letters, one vertical line
// Yline[0] goes into row 7, 0xBE
// Yline[1] goes into row 6, 0xBD
// Yline[2] goes into row 5, 0xBC
// Yline[3] goes into row 4, 0xBB
// Yline[4] goes into row 3, 0xBA
// Yline[5] goes into row 2, 0xB9
void lcdFillYline(unsigned char mask, char *label){
unsigned short i,j,cnt; unsigned char letter,data,ymask;
// i is index into font table
// j is index into Yline
// cnt is used to move 5 pixels from font table into Yline
// letter is one ASCII from input string
// data is information from font table
// j with ymask allows bit access to 48 bits of Yline
  for(j=0; j<6; j++){
    Yline[j] = 0;     // default is blank
  }
  j = 0;              // index into Yline
  ymask = 0x80;
  do{
    letter = *label++;            // next character
    if(letter == 0) return;       // done
    if(letter<32) letter = 32;    // nonprinting
    if(letter>126) letter = 32;   // nonprinting
    i = 5*(letter-32);            // index into font table
    for(cnt=5; cnt>0; cnt--){
      data = Font[i];        // each character is 5 pixels wide
      i++;
      if(data&mask){
        Yline[j] |= ymask;   // set bit
      }
      ymask = ymask >>1;     // 80,40,20,10,8,4,2,1
      if(ymask == 0){
        ymask = 0x80;        // next byte of Yline
        j++;
      }
    }
    ymask = ymask >>1;       // add intercharacter space
    if(ymask == 0){
      ymask = 0x80;          // next byte of Yline
      j++;
    }
  }
  while(j<6);
}

//-----------------------LCD_PlotYaxis-----------------------
// Draw Y axis
// Input: min, center, max, resolution, label, and number of hash marks
//        min, center, max are the integer part ranging from -99 to +99
//        resolution is 0,1,2 where to put decimal point
//                      any other value means no numbers are displayed
//  number->    -99    -1    0    1    99
// 0  shown as -99.   -1.    0.   1.   99.
// 1  shown as -9.9  -0.1   0.0  0.1   9.9
// 2  shown as -.99  -.01   .00  .01   .99
//        label is an ASCII string, up to 8 characters
// Output: none
// errors: must be called once, before calling Plot
void LCD_PlotYaxis(short min, short center, short max,
  unsigned short resolution, char *label){
  unsigned char page,column; // access to Ylabel area
  unsigned short j;          // index into Yline
  unsigned char mask;        // used to get one line at a time
  if(OpenFlag==0){
    return;  // not open
  }
  if((min > max)||(center < min)||(center > max)){
    return;  // bad parameters
  }
  if((min < -99)||(max > 99)){
    return;  // bad parameters
  }
  YaxisResolution = resolution;
  CS = LEFT;         // paint all numbers on left side
  column = 0x40;
  for(mask=0x01; mask<0x80; mask = mask<<1){
    // mask goes 1,2,4,8,10,20,40
    lcdFillYline(mask,label); // get 48 bits
    page = 0xBE;     // row 7
    for(j=0; j<6; j++){
      lcdCmd(page);  // row 7,6,5,4,3,2
      lcdCmd(column);// column 1,2,3,4,5,6,7
      lcdData(Yline[j]);
      page--;        // BE, BD, BC, BB, BA, B9
    }
    column++;        // 40,41,42,43,44,45,46
  }

  TinyDown = 0;      // shift down no pixels
  lcdCmd(0xB9);      // Page 2
  lcdCmd(0x48);      // byte number = 8
  lcdTinyOutFix2(max);
  TinyDown = 2;      // shift down two pixels
  lcdCmd(0xBB);      // Page 4
  lcdCmd(0x48);      // byte number = 8
  lcdTinyOutFix2(center);
  TinyDown = 3;      // shift down three pixels
  lcdCmd(0xBD);      // Page 6
  lcdCmd(0x48);      // byte number = 8
  lcdTinyOutFix2(min);

}

//-----------------------LCD_PlotClear-----------------------
// Clear plot window,
// Input: min max, specify range of Y axis
// plotPixel number of data points drawn into the same X-axis pixel
// 4 means it takes 400 calls to LCD_Plot to draw one sweep
// plotPixel can range from 1 to 100
// Output: none
// errors: must be called once, before calling Plot
void LCD_PlotClear(short min, short max, unsigned char plotPixel){
int i,j;  unsigned char page;
  if(OpenFlag==0){
    return;  // not open
  }
  if((plotPixel<1)||(plotPixel>100)) plotPixel=1;
  PlotPixel = plotPixel;
  RangeY = max-min;
  MinY = min;
  SubCount = PlotPixel;
  Xcolumn = 0x56;    // byte number 22
  bRightx = NONE;    // to be placed into CS2, start on left
  bLeftx = LEFT;     // to be placed into CS1
  for(j=1; j<6; j++){
    Yline[j] = 0;    // default is blank
  }
  CS = LEFT;         // left enable
  lcdCmd(0xB9);      // Page 2
  lcdCmd(0x55);      // byte number = 21
  lcdData(0xFF);     // Yaxis line
  lcdData(0x01); Yline[5]=0x01;    // top hash
  for(i=0;i<41;i++){
    lcdData(0);
  }
  lcdCmd(0xBA);      // Page 3
  lcdCmd(0x55);      // byte number = 21
  lcdData(0xFF);     // Yaxis line
  lcdData(0x02); Yline[4]=0x02;    // 75% hash
  for(i=0;i<41;i++){
    lcdData(0);
  }
  lcdCmd(0xBB);      // Page 4
  lcdCmd(0x55);      // byte number = 21
  lcdData(0xFF);     // Yaxis line
  lcdData(0x10); Yline[3]=0x10;    // 50% hash
  for(i=0;i<41;i++){
    lcdData(0);
  }
  lcdCmd(0xBC);      // Page 5
  lcdCmd(0x55);      // byte number = 21
  lcdData(0xFF);     // Yaxis line
  lcdData(0x40); Yline[2]=0x40;    // 25% hash
  for(i=0;i<41;i++){
    lcdData(0);
  }
  lcdCmd(0xBD);      // Page 6
  lcdCmd(0x55);      // byte number = 21
  lcdData(0xFF);     // Yaxis line
  Yline[1] = 0x80;   // Xaxis
  for(j=0; j<4; j++){
    for(i=0;i<9;i++){
      lcdData(0x80);  // X axis
    }
    lcdData(0xC0);    // X axis and hash
  }
  lcdData(0x80);      // X axis
  lcdData(0x80);      // X axis

  CS = RIGHT;          // right enable
  for(page=0xB9;page<0xBD; page++){
    lcdCmd(page);      // Page 2,3,4,5
    lcdCmd(0x40);      // byte number = 0
    for(i=0;i<58;i++){
      lcdData(0);
    }
  }
  lcdCmd(0xBD);      // Page 6
  lcdCmd(0x40);      // byte number = 0
  for(i=0;i<8;i++){
    lcdData(0x80);    // X axis
  }
  lcdData(0xC0);    // X axis and hash
  for(j=0; j<5; j++){
    for(i=0;i<9;i++){
      lcdData(0x80);  // X axis
    }
    lcdData(0xC0);    // X axis and hash
  }
}
const unsigned char BitMask[8]={
  0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01
};

//-----------------------LCD_Plot-----------------------
// Plot one data point
// Input: data between min and max
// Output: none
// errors: must call LCD_PlotClear first
void LCD_Plot(long data){
unsigned short pixel;
unsigned short j;  // index into Yline
unsigned char page;
  if(OpenFlag==0){
    return;        // not open
  }
  if(data<MinY)data=MinY;
  data = 40*(data-MinY);
  pixel = data/RangeY;  // 0 to 39, truncate down
/*  asm ldd  data
  asm subd MinY    // must be positive
  asm ldy  #40
  asm emul         // 32-bit Y:D is 40*(data-MinY)
  asm ldx  RangeY
  asm ediv         // 16-bit Y is 40*(data-MinY)/RangeY
  asm sty  pixel   // should be 0 to 40*/
  if(pixel > 39){
    pixel = 39;
  }
  j = 1+(pixel/8); // 1,2,3,4,5
  Yline[j] |= BitMask[pixel&0x07];
  SubCount--;
  if(SubCount==0){
    // output 5 bytes to LCD
    CS1 = bLeftx;
    CS2 = bRightx;      // select correct side
    page = 0xBD;        // start in row 6
    for(j=1; j<6; j++){
      lcdCmd(page);     // row 6,5,4,3,2
      lcdCmd(Xcolumn);  // column X parameter
      lcdData(Yline[j]);// 5 bytes is 40 pixels
      page--;           // BE, BD, BC, BB, BA, B9
    }

    // setup for next line
    Xcolumn++;
    Yline[5] = 0x00;
    Yline[4] = 0x00;
    Yline[3] = 0x00;
    Yline[2] = 0x00;
    Yline[1] = 0x80;    // bottom bit is x axis
    if(bLeftx){
      if(Xcolumn == 0x80){
        Xcolumn = 0x40; // beginning of right side
        bLeftx = NONE;
        bRightx = RIGHT;// switch to right side
      } else{
        switch(Xcolumn){
          case 0x5F:
          case 0x69:
          case 0x73:
          case 0x7D: Yline[1] = 0xC0; break;  // hash
        }
      }
    } else{              // on right
      if(Xcolumn == 0x7A){
        Xcolumn = 0x56;  // beginning of left side
        bLeftx = LEFT;
        bRightx = NONE;  // switch to left side
        Yline[5] = 0x01;    // top hash
        Yline[4] = 0x02;    // 75% hash
        Yline[3] = 0x10;    // 50% hash
        Yline[2] = 0x40;    // 25% hash
        Yline[1] = 0x80;    // X axis
      } else{
        switch(Xcolumn){
          case 0x47:
          case 0x51:
          case 0x5B:
          case 0x65:
          case 0x6F:
          case 0x77: Yline[1] = 0xC0; break;  // hash
        }
      }
    }
    SubCount = PlotPixel;
  }
}

void LCD_Test(void){
#if TEST
  LCD_Clear(0xFF);
// *********pause here*************

  LCD_Clear(0);
  LCD_GoTo(1,1);  LCD_OutChar('1');
  LCD_GoTo(2,2);  LCD_OutChar('2');
  LCD_GoTo(3,3);  LCD_OutChar('3');
  LCD_GoTo(4,4);  LCD_OutChar('4');
  LCD_GoTo(5,5);  LCD_OutChar('5');
  LCD_GoTo(6,6);  LCD_OutChar('6');
  LCD_GoTo(7,7);  LCD_OutChar('7');
  LCD_GoTo(8,8);  LCD_OutChar('8');
  LCD_GoTo(1,9);  LCD_OutChar('9');
  LCD_GoTo(2,10); LCD_OutChar('0');
  LCD_GoTo(3,11); LCD_OutChar('1');
  LCD_GoTo(4,12); LCD_OutChar('2');
  LCD_GoTo(5,13); LCD_OutChar('3');
  LCD_GoTo(6,14); LCD_OutChar('4');
  LCD_GoTo(7,15); LCD_OutChar('5');
  LCD_GoTo(8,16); LCD_OutChar('6');
  LCD_GoTo(1,17); LCD_OutChar('7');
  LCD_GoTo(2,18); LCD_OutChar('8');
  LCD_GoTo(3,19); LCD_OutChar('9');
  LCD_GoTo(4,20); LCD_OutChar('0');
  LCD_GoTo(5,21); LCD_OutChar('1');
// *********pause here*************

  LCD_Clear(0);
// *********pause here*************

  LCD_VTest(0xFF,0x00); // BUG STILL HERE
// *********pause here*************

  LCD_VTest(0x00,0xFF);
// *********pause here*************

  LCD_VTest(0xAA,0x08);
// *********pause here*************

  LCD_VTest(0x01,0x02);
// *********pause here*************

  LCD_GoTo(8,1);    LCD_OutString("098765432109876543210");
  LCD_GoTo(7,1);    LCD_OutString("qwertyuiopasdfghjklzx");
  LCD_GoTo(6,1);    LCD_OutString("012345678901234567890");
  LCD_GoTo(5,1);    LCD_OutString("abcdefghijklmnopqrstu");
  LCD_GoTo(4,1);    LCD_OutString("098765432109876543210");
  LCD_GoTo(3,1);    LCD_OutString("qwertyuiopasdfghjklzx");
  LCD_GoTo(2,1);    LCD_OutString("012345678901234567890");
  LCD_GoTo(1,1);    LCD_OutString("abcdefghijklmnopqrstu");
// *********pause here*************

  LCD_GoTo(4,1);    LCD_OutString("0 9 8 7 6 5 4 3 2 1 0");
  LCD_GoTo(3,1);    LCD_OutString("q w e r t y u i o p a");
  LCD_GoTo(2,1);    LCD_OutString(" 0 1 2 3 4 5 6 7 8 9 ");
  LCD_GoTo(1,1);    LCD_OutString(" a  b  c  d  e  f  g ");
  LCD_GoTo(8,1);    LCD_OutString("0 9 8 7 6 5 4 3 2 1 0");
  LCD_GoTo(7,1);    LCD_OutString("q w e r t y u i o p a");
  LCD_GoTo(6,1);    LCD_OutString(" 0 1 2 3 4 5 6 7 8 9 ");
  LCD_GoTo(5,1);    LCD_OutString(" a  b  c  d  e  f  g ");
// *********pause here*************

  LCD_DrawImageTest();
#endif
}
