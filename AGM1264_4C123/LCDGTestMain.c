/* ******************** LCDGTestMain.c *************************
 * AGM1264 example
 * ************************************************************ */

// Jonathan W. Valvano 11/7/11
// modified to run on LM4F120 launchpad on 10/8/12
// compiled on TM4C123 on 9/11/13
// May 3, 2015
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
// 10k pot from pin 18 to ground, with center to pin 3

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

// references   http://www.azdisplays.com/prod/g1264f.php
// sample code  http://www.azdisplays.com/PDF/agm1264f_code.pdf
// data sheet   http://www.azdisplays.com/PDF/agm1264f.pdf

// hardware specific to this example:
// PD0 may still be connected to PB6 on launchpad board, therefore unused
// PD1 may still be connected to PB7 on launchpad board, therefore unused
#include <stdint.h>
#include "LCDG.h"
#include "Systick.h"
#include "PLL.h"

unsigned short ADCsample; // ADC sample, 0 to 1023
unsigned long Voltage;    // 0.01 volts, 0 to 300
unsigned short ADCcount;  // 0 to 2999
unsigned short StartTime; // in seconds
int main(void){
  PLL_Init(Bus50MHz);  // initialize 50 MHz PLL 
  LCD_Init();  // initialize LCD and SysTick

  LCD_Test();  // to run this, set TEST=1 inside LCDG.c, single step through
  LCD_Clear(0);

  LCD_OutString("Jonathan Valvano");
  LCD_PlotYaxis(0, 15, 30, 1, "Voltage");
  LCD_PlotClear(0, 300, 30); //30*100=3000 points/sweep
  StartTime = 0; ADCcount = 0;
  LCD_PlotXaxis(StartTime, StartTime+30, 0, "Time (sec)");
  LCD_GoTo(8,1);
  LCD_OutFix2(0);
  LCD_OutString(" volts");
  for(;;) {
    SysTick_Wait10ms(1);
    ADCsample = (ADCsample+1)&0x3FF;
    Voltage = (300*ADCsample)/1023;
    LCD_Plot(Voltage);
    ADCcount++;             // 0 to 2999
    if(ADCcount == 3000){   // end of sweep??
      StartTime = StartTime+30;
      if(StartTime>900) StartTime=0;
      LCD_PlotXaxis(StartTime, StartTime+30, 0, "Time (sec)");
      LCD_GoTo(8,1);
      LCD_OutFix2(Voltage);
      ADCcount = 0;
    }
  }
}

