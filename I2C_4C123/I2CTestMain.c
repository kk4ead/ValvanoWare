// I2CTestMain.c
// Runs on LM4F120/TM4C123
// Test the functions provided in I2C0.c by periodically sampling
// the temperature, parsing the result, and sending it to the UART.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
   Section 8.6.4 Programs 8.5, 8.6 and 8.7

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

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
// I2C0SCL connected to PB2 and to pin 4 of HMC6352 compass or pin 3 of TMP102 thermometer
// I2C0SDA connected to PB3 and to pin 3 of HMC6352 compass or pin 2 of TMP102 thermometer
// SCL and SDA lines pulled to +3.3 V with 10 k resistors (part of breakout module)
// ADD0 pin of TMP102 thermometer connected to GND
#include <stdint.h>
#include "I2C0.h"
#include "PLL.h"
#include "UART.h"

// For debug purposes, this program may peek at the I2C0 Master
// Control/Status Register to try to provide a more meaningful
// diagnostic message in the event of an error.  The rest of the
// interface with the I2C hardware occurs through the functions
// in I2C0.c.
#define I2C0_MASTER_MCS_R       (*((volatile unsigned long *)0x40020004))

// DEBUGPRINTS==0 configures for no test prints, other value prints test text
// This tests the math used to convert the raw temperature value
// from the thermometer to a string that is displayed.  Verify
// that the left and right columns are the same.
#define DEBUGPRINTS 0
// DEBUGWAIT is time between test prints as a parameter for the Delay() function
// DEBUGWAIT==16,666,666 delays for 1 second between lines
// This is useful if the computer terminal program has limited
// screen or log space to prevent the text from scrolling too
// fast.
// Definition has no effect if test prints are off.
#define DEBUGWAIT   16666666

// delay function for testing from sysctl.c
// which delays 3*ulCount cycles
#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  void Delay(unsigned long ulCount){
  __asm (  "    subs    r0, #1\n"
      "    bne     Delay\n"
      "    bx      lr\n");
}

#else
  //Keil uVision Code
  __asm void
  Delay(unsigned long ulCount)
  {
    subs    r0, #1
    bne     Delay
    bx      lr
  }

#endif
// function parses raw 16-bit number from temperature sensor in form:
// rawdata[0] = 0
// rawdata[15:8] 8-bit signed integer temperature
// rawdata[7:4] 4-bit unsigned fractional temperature (units 1/16 degree C)
//  or
// rawdata[0] = 1
// rawdata[15:7] 9-bit signed integer temperature
// rawdata[6:3] 4-bit unsigned fractional temperature (units 1/16 degree C)
void parseTemp(unsigned short rawData, int * tempInt, int * tempFra){
  if(rawData&0x0001){  // 13-bit mode
    *tempInt = rawData>>7;
    if(rawData&0x8000){// negative value
      *tempFra = (16 - ((rawData>>3)&0xF))*10000/16;
                                                 // treat as 9-bit signed
      *tempInt = (*tempInt) - 1512;              // subtract 512 to make integer portion signed
                                                 // subtract extra 1,000 so integer portion is
                                                 // still negative in the case of -0.XXXX
                                                 // (never display thousands digit)
      if(((*tempFra) > 0) && (*tempFra) < 10000){// fractional part "borrows" from integer part
        *tempInt = (*tempInt) + 1;
      }
    }
    else{
      *tempFra = ((rawData>>3)&0xF)*10000/16;
    }
  }
  else{
    *tempInt = rawData>>8;
    if(rawData&0x8000){// negative value
      *tempFra = (16 - ((rawData>>4)&0xF))*10000/16;
                                                 // treat as 8-bit signed
      *tempInt = (*tempInt) - 1256;              // subtract 256 to make integer portion signed
                                                 // subtract extra 1,000 so integer portion is
                                                 // still negative in the case of -0.XXXX
                                                 // (never display thousands digit)
      if(((*tempFra) > 0) && (*tempFra) < 10000){// decimal part "borrows" from integer part
        *tempInt = (*tempInt) + 1;
      }
    }
    else{
      *tempFra = ((rawData>>4)&0xF)*10000/16;
    }
  }
}
// function sends temperature integer and decimal components to UART
// in form:
// XXX.XXXX or -XXX.XXXX
// tempInt is signed integer value of temperature
// tempFra is unsigned fractional value of temperature (units 1/10000 degree C)
void displayTemp(int * tempInt, int * tempFra){
  uint32_t index = 0;               // string index
  char str[10];                           // holds 9 characters
  // first character is '-' if negative
  if((*tempInt) < 0){
    *tempInt = -1*(*tempInt);
    str[index] = '-';
    index = index + 1;
  }
  // next character is hundreds digit if not zero
  if(((*tempInt)/100)%10 != 0){
    str[index] = (((*tempInt)/100)%10) + '0';
    index = index + 1;
  // hundreds digit is not zero so next character is tens digit
    str[index] = (((*tempInt)/10)%10) + '0';
    index = index + 1;
  }
  // hundreds digit is zero so next character is tens digit only if not zero
  else if(((*tempInt)/10)%10 != 0){
    str[index] = (((*tempInt)/10)%10) + '0';
    index = index + 1;
  }
  // next character is ones digit
  str[index] = ((*tempInt)%10) + '0';
  index = index + 1;
  // next character is '.'
  str[index] = '.';
  index = index + 1;
  // next character is tenths digit
  str[index] = (((*tempFra)/1000)%10) + '0';
  index = index + 1;
  // next character is hundredths digit
  str[index] = (((*tempFra)/100)%10) + '0';
  index = index + 1;
  // next character is thousandths digit
  str[index] = (((*tempFra)/10)%10) + '0';
  index = index + 1;
  // next character is ten thousandths digit
  str[index] = ((*tempFra)%10) + '0';
  index = index + 1;
  // fill in any remaining characters with ' '
  while(index < 9){
    str[index] = ' ';
    index = index + 1;
  }
  // final character is null terminater
  str[index] = 0;
  // send string to UART
  UART_OutChar('\r');
  UART_OutString(str);
}
//volatile uint16_t heading = 0;
//volatile uint8_t controlReg = 0;
int main(void){
  unsigned short rawData = 0;             // 16-bit data straight from thermometer
  int tempInt = 0;                        // integer value of temperature (-128 to 127)
  int tempFra = 0;                        // fractional value of temperature (0 to 9375)
  PLL_Init(Bus80MHz);
  UART_Init();
  I2C_Init();
                                          // write commands to 0x48 (ADDR to ground)
  I2C_Send1(0x48, 1);                     // use command 1 to set pointer to config (Figure 7.XX chapter7-10-1.ulb)
                                          // read from 0x48 to get data
  if(I2C_Recv2(0x48) == 0x60A0){          // expected 0x60A0 as power-on default (Figure 7.XX chapter7-10-2.ulb)
    UART_OutString("\r\nTest Passed\r\n");
  }
  else{
    if(I2C0_MASTER_MCS_R&0x02){
      UART_OutString("\r\nNo Acknowledge\r\n");
    }
    else{
      UART_OutString("\r\nTest Failed\r\n");
    }
  }
//test display and number parser (can safely be skipped)
#if DEBUGPRINTS
  UART_OutString("Test:     Expected:\r\n");
  parseTemp(0x7FF0, &tempInt, &tempFra);  // expect 127.9375
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 127.9375\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x6400, &tempInt, &tempFra);  // expect 100.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 100.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x3200, &tempInt, &tempFra);  // expect 50.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 50.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x1900, &tempInt, &tempFra);  // expect 25.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 25.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x0300, &tempInt, &tempFra);  // expect 3.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 3.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x0100, &tempInt, &tempFra);  // expect 1.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 1.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x0040, &tempInt, &tempFra);  // expect 0.2500
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 0.2500\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x0060, &tempInt, &tempFra);  // expect 0.3750
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 0.3750\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x00F0, &tempInt, &tempFra);  // expect 0.9375
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 0.9375\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x0000, &tempInt, &tempFra);  // expect 0.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" 0.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0xFFC0, &tempInt, &tempFra);  // expect -0.2500
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" -0.2500\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0xE700, &tempInt, &tempFra);  // expect -25.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" -25.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0xC900, &tempInt, &tempFra);  // expect -55.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" -55.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0xFF00, &tempInt, &tempFra);  // expect -1.0000
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" -1.0000\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x9D60, &tempInt, &tempFra);  // expect -98.6250
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" -98.6250\r\n");
  Delay(DEBUGWAIT);                       // short delay
  parseTemp(0x8490, &tempInt, &tempFra);  // expect -123.4375
  displayTemp(&tempInt, &tempFra);
  UART_OutString(" -123.4375\r\n");
  Delay(DEBUGWAIT);                       // short delay
#endif
//done testing
  UART_OutString("Current Temp:\r\n");
                                          // write commands to 0x48 (ADDR to ground)
  I2C_Send1(0x48, 0);                     // use command 0 to set pointer to temperature
  while(1){
                                          // read from 0x48 to get temperature
    rawData = I2C_Recv2(0x48);            // temperature data stored in 12 or 13 MSBs (Figure 7.XX chapter7-10-3.ulb, reply 0x1660, 22.375 C)
    parseTemp(rawData, &tempInt, &tempFra);
    displayTemp(&tempInt, &tempFra);
    Delay(4166667);                       // wait 250 ms to sample at ~4 Hz
  }
/*  while(1){
                                          // write commands to 0x42
    I2C_Send1(0x42, 'A');                 // use command 'A' to sample
    Delay(100000);                        // wait 6,000 us for sampling to finish
                                          // read from 0x43 to get data
    heading = I2C_Recv2(0x43);            // 0 to 3599 (units: 1/10 degree)
// test sending multiple bytes and receiving single byte
                                          // write commands to 0x42
    I2C_Send2(0x42, 'g', 0x74);           // use command 'g' to read from RAM 0x74
    Delay(1167);                          // wait 70 us for RAM access to finish
    controlReg = I2C_Recv(0x43);          // expected 0x50 as default
    Delay(16666666);                      // wait 1 sec
  }*/
}
