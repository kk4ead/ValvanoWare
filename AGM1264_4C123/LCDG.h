//*****************LCDG.h***************************************
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


// to use it as an 8-line by 21-character display
// initialize it once using
//      LCD_Init
// clear screen with
//      LCD_Clear
// set cursor position using
//      LCD_GoTo
// place ASCII on the screen using
//      LCD_OutChar
//      LCD_OutString
//      LCD_OutDec
//      LCD_OutFix1
//      LCD_OutFix2
//      LCD_OutFix3

// ********* LCD_Init***********
// Initialize AGM1264F 128-bit by 64-bit graphics display
// activates SysTick at 50 MHz, assumes PLL active
// Input: none
// Output: none
// does not clear the display
void LCD_Init(void);

// ********* LCD_Clear***********
// Clear the entire 1024 byte (8192 bit) image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: value to write into all bytes of display RAM
// Output: none
// e.g., LCD_Clear(0);  // makes all pixels off
void LCD_Clear(unsigned char data);

//-----------------------LCD_GoTo-----------------------
// Move cursor
// Input: line number is 1 to 8, column from 1 to 21
// Output: none
// errors: it will ignore legal addresses
void LCD_GoTo(int line, int column);

// ********* LCD_OutChar***********
// Output ASCII character on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: 7-bit ASCII to display
// Output: none
// letter must be between 32 and 127 inclusive
// execute LCD_GoTo to specify cursor location
void LCD_OutChar(unsigned char letter);

//---------------------LCD_OutString--------------
// Display String
// Input: pointer to NULL-terminationed ASCII string
// Output: none
void LCD_OutString(char *pt);

//-----------------------LCD_OutDec-----------------------
// Output a 16-bit number in unsigned decimal format
// Input: 16-bit unsigned number
// Output: none
// fixed size 5 digits of output, right justified
void LCD_OutDec(unsigned short n);

//-----------------------LCD_OutSDec-----------------------
// Output a 16-bit number in signed decimal format
// Input: 16-bit signed number
// Output: none
// fixed size 6 digits of output, right justified
void LCD_OutSDec(short n);

//-----------------------LCD_OutFix1-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.1
// Input: 16-bit unsigned number
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 1234.5
void LCD_OutFix1(unsigned short n);

//-----------------------LCD_OutFix2-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01
// Input: 16-bit unsigned number
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 123.45
void LCD_OutFix2(unsigned short n);

//-----------------------LCD_OutFix2b-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01
// Input: 16-bit unsigned number, 0 to 9999
// Output: none
// fixed size is 5 characters of output, right justified
// if input is 2345, then display is 23.45
void LCD_OutFix2b(unsigned short n);

//-----------------------LCD_OutFix3-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.001
// Input: 16-bit unsigned number
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 12.345
void LCD_OutFix3(unsigned short n);

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
  unsigned short resolution, char *label);

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
  unsigned short resolution, char *label);

//-----------------------LCD_PlotClear-----------------------
// Clear plot window,
// Input: min max, specify range of Y axis
// plotPixel number of data points drawn into the same X-axis pixel
// plotPixel can be any number from 1 to 100
// 4 means it takes 400 calls to LCD_Plot to draw one sweep
// Output: none
// errors: must be called once, before calling Plot
void LCD_PlotClear(short min, short max, unsigned char plotPixel);

//-----------------------LCD_Plot-----------------------
// Plot one data point
// Input: data between min and max
// Output: none
// errors: must call LCD_PlotClear first
void LCD_Plot(long data);

// ********* LCD_VTest***********
// Alternate num1,num2 output image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: num1, num2 are pattern to draw
// Output: none
void LCD_VTest(unsigned char num1, unsigned char num2);

// ********* LCD_DrawImage***********
// Draw an entire 1024 byte (8192 bit) image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: pointer to 1024 bytes of data
// Output: none
void LCD_DrawImage(const unsigned char *pt);

// ********* LCD_DrawImageTest***********
// Draw test image on the
//    AGM1264F 128-bit by 64-bit graphics display
// Input: none
// Output: none
void LCD_DrawImageTest(void);

// ********* LCD_Test***********
// Step over, through this test program
//    AGM1264F 128-bit by 64-bit graphics display
// Input: none
// Output: none
void LCD_Test(void);
