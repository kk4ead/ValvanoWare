// 74HC165.c
// Runs on TM4C123
// Use SSI0 to receive an 8-bit code from the 74HC165.
// Output port expander
// If running at 80 MHz change SSI0_CPSR_R to 4
// Daniel Valvano
// July 17, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 7.3

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
// TM4C123      74HC165
//   +3.3       pin 16 Vcc powewr
//   Gnd        pin 8  ground
//   Gnd        pin 15 CLK INH (allow clock to operate)
//   Gnd        pin 10 SER (data in is NA)
// PA2 SSI0Clk  pin 2  SCK
// PA3 GPIO     pin 1  SH/LD (0 for load, 1 for shift)
// PA4 SSI0Rx   pin 9  Qh (data shifted out)
//     nc       pin 7  Qh' is not needed

// Port         74HC165
// bit 7 (msb)  pin 6  H
// bit 6        pin 5  G
// bit 5        pin 4  F
// bit 4        pin 3  E
// bit 3        pin 14 D
// bit 2        pin 13 C
// bit 1        pin 12 B
// bit 0 (LSB)  pin 11 A


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define PA3   (*((volatile uint32_t *)0x40004020))

//********Port_Init*****************
// Initialize 74HC165 serial shift register
// inputs:  none
// outputs: none
// assumes: system clock rate less than or equal to 50 MHz
// 74HC165 clocks out on rise, TM4C123 clocks in on fall
void Port_Init(void){
  SYSCTL_RCGCSSI_R |= 0x01;       // activate SSI0
  SYSCTL_RCGCGPIO_R |= 0x01;      // activate port A
  while((SYSCTL_PRGPIO_R&0x01) == 0){};// ready?
  GPIO_PORTA_AFSEL_R |= 0x14;     // enable alt funct on PA2,4
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFF0F0FF)+0x00020200;
  GPIO_PORTA_AMSEL_R = 0;         // disable analog functionality on PA
  GPIO_PORTA_DEN_R |= 0x1C;       // enable digital I/O on PA2,3,4
  GPIO_PORTA_DIR_R |= 0x08;       // output on PA3
  SSI0_CR1_R = 0x00000000;        // disable SSI, master mode
  SSI0_CPSR_R = 0x02;             // 8 MHz SSIClk
  SSI0_CR0_R = (SSI0_CR0_R&~0xFFF0)|0x40;  // SCR = 0, SPH = 0, SPO = 1 Freescale
  SSI0_CR0_R = (SSI0_CR0_R&~0x0F)|0x07; // 8-bit data
  SSI0_CR1_R |= 0x00000002;       // enable SSI
  PA3 = 0;   // load mode
}

//********Port_In*****************
// Receive data from 74HC165 8-bit port
// inputs:  none
// outputs: data (0 to 255)
uint8_t Port_In(void){uint8_t data;
  PA3 = 0x08;   // enable shifting
  while((SSI0_SR_R&0x02)==0){}; // wait until room in FIFO
  SSI0_DR_R = 0;                // data out to start
  while((SSI0_SR_R&0x04)==0){}; // wait for response
  data = SSI0_DR_R;
  PA3 = 0;   // load mode
  return data;
}

