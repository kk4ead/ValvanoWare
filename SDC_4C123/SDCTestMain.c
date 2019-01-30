// SDCTestMain.c
// Runs on TM4C123/LM4F120
// Test Secure Digital Card read/write interface by writing test
// data, reading them back, and verifying that they match as
// expected.  Running the unformatted file tests will destroy
// any formatting already on the disk.  The formatted file tests
// will not work unless the disk has already been formatted.
// Valvano
// March 17, 2014

/* This example accompanies the books
   Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers, Volume 3,  
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2013

   Volume 3, Program 6.3, section 6.6   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

 Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// hardware connections
// **********ST7735 TFT and SDC*******************
// ST7735
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) connected to PA4 (SSI0Rx)
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss) <- GPIO high to disable TFT
// CARD_CS (pin 5) connected to PD7 GPIO output 
// Data/Command (pin 4) connected to PA6 (GPIO)<- GPIO low not using TFT
// RESET (pin 3) connected to PA7 (GPIO)<- GPIO high to disable TFT
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

// **********wide.hk ST7735R*******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - PD7 SDC_CS, active low to enable SDC
// MISO - PA4 MISO SPI data from SDC to microcontroller
// SDA  – (NC) I2C data for ADXL345 accelerometer
// SCL  – (NC) I2C clock for ADXL345 accelerometer
// SDO  – (NC) I2C alternate address for ADXL345 accelerometer
// Backlight + - Light, backlight connected to +3.3 V
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "edisk.h"

// PF1 is profile for eDisk_WriteBlock
// PF2 on for running, low for error
// PF3 is profile for eDisk_ReadBlock
unsigned char buffer[512];
#define MAXBLOCKS 100
#define PF1  (*((volatile unsigned long *)0x40025008))
#define PF2  (*((volatile unsigned long *)0x40025010))
#define PF3  (*((volatile unsigned long *)0x40025020))

void diskError(char* errtype, unsigned long n){
  PF2 = 0x00;      // turn LED off to indicate error
  while(1){};
}
void TestDisk(void){  DSTATUS result;  unsigned short block;  int i; unsigned long n;
  // simple test of eDisk
  result = eDisk_Init(0);  // initialize disk
  if(result) diskError("eDisk_Init",result);
  n = 1;    // seed
  for(block = 0; block < MAXBLOCKS; block++){
    for(i=0;i<512;i++){
      n = (16807*n)%2147483647; // pseudo random sequence
      buffer[i] = 0xFF&n;
    }
    PF1 = 0x02;
    if(eDisk_WriteBlock(buffer,block))diskError("eDisk_WriteBlock",block); // save to disk
    PF1 = 0;
  }
  n = 1;  // reseed, start over to get the same sequence
  for(block = 0; block < MAXBLOCKS; block++){
    PF3 = 0x08;
    if(eDisk_ReadBlock(buffer,block))diskError("eDisk_ReadBlock",block); // read from disk
    PF3 = 0;
    for(i=0;i<512;i++){
      n = (16807*n)%2147483647; // pseudo random sequence
      if(buffer[i] != (0xFF&n)){
          PF2 = 0x00;   // turn LED off to indicate error
      }
    }
  }
}
void PortF_Init(void){  unsigned long volatile delay;
  SYSCTL_RCGCGPIO_R |= 0x20; // activate port F
  delay = SYSCTL_RCGCGPIO_R;
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
  GPIO_PORTF_CR_R |= 0x1F;           // allow changes to PF4-0       
  GPIO_PORTF_DIR_R = 0x0E;     // make PF3-1 output (PF3-1 built-in LEDs),PF4,0 input
  GPIO_PORTF_PUR_R = 0x11;     // PF4,0 have pullup
  GPIO_PORTF_AFSEL_R = 0x00;   // disable alt funct on PF4-0
  GPIO_PORTF_DEN_R = 0x1F;     // enable digital I/O on PF4-0
  GPIO_PORTF_PCTL_R = 0x00000000;
  GPIO_PORTF_AMSEL_R = 0;      // disable analog functionality on PF
}

int main(void){ int i=0;               
  PLL_Init(Bus80MHz);    // bus clock at 80 MHz
  PortF_Init();  // LaunchPad switches and LED
  PF2 = 0x04;    // turn blue LED on
// *******************unformatted file tests********************
  while(1){
    TestDisk();
    i = i + 1;
  }
// ****************end of unformatted file tests****************
}
