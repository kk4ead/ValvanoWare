// MAX5353.c
// Runs on LM4F120/TM4C123
// Use SSI0 to send a 16-bit code to the MAX5353.
// Daniel Valvano
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

// SSI0Clk (SCLK, pin 4) connected to PA2
// SSI0Fss (!CS, pin 2) connected to PA3
// SSI0Tx (DIN, pin 3) connected to PA5
// see Volume 2 Figure 7.19 for complete schematic
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"


#define SSI_CR0_SCR_M           0x0000FF00  // SSI Serial Clock Rate
#define SSI_CR0_SPH             0x00000080  // SSI Serial Clock Phase
#define SSI_CR0_SPO             0x00000040  // SSI Serial Clock Polarity
#define SSI_CR0_FRF_M           0x00000030  // SSI Frame Format Select
#define SSI_CR0_FRF_MOTO        0x00000000  // Freescale SPI Frame Format
#define SSI_CR0_DSS_M           0x0000000F  // SSI Data Size Select
#define SSI_CR0_DSS_16          0x0000000F  // 16-bit data
#define SSI_CR1_MS              0x00000004  // SSI Master/Slave Select
#define SSI_CR1_SSE             0x00000002  // SSI Synchronous Serial Port
#define SSI_SR_RNE              0x00000004  // SSI Receive FIFO Not Empty
#define SSI_SR_TNF              0x00000002  // SSI Transmit FIFO Not Full
                                            // Enable
#define SSI_CPSR_CPSDVSR_M      0x000000FF  // SSI Clock Prescale Divisor
#define SSI_DMACTL_TXDMAE       0x00000002  // Transmit DMA Enable
#define SSI_DMACTL_RXDMAE       0x00000001  // Receive DMA Enable


//********DAC_Init*****************
// Initialize MAX5353 12-bit DAC
// inputs:  initial voltage output (0 to 4095)
// outputs: none
// assumes: system clock rate of 80 MHz
void DAC_Init(uint16_t data){
  volatile uint32_t delay;
  SYSCTL_RCGCSSI_R |= 0x01;  // activate SSI0
  SYSCTL_RCGCGPIO_R |= 0x01; // activate port A
  delay = SYSCTL_RCGC2_R;               // allow time to finish activating
  GPIO_PORTA_AFSEL_R |= 0x2C;           // enable alt funct on PA2,3,5
  GPIO_PORTA_DEN_R |= 0x2C;             // enable digital I/O on PA2,3,5
                                        // configure PA2,3,5 as SSI
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0F00FF)+0x00202200;
  GPIO_PORTA_AMSEL_R = 0;               // disable analog functionality on PA
  SSI0_CR1_R &= ~SSI_CR1_SSE;           // disable SSI
  SSI0_CR1_R &= ~SSI_CR1_MS;            // master mode
                                        // clock divider for 8 MHz SSIClk (assumes 80 MHz PLL)
  SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+10;
  SSI0_CR0_R &= ~(SSI_CR0_SCR_M |       // SCR = 0 (8 Mbps data rate)
                  SSI_CR0_SPH |         // SPH = 0
                  SSI_CR0_SPO);         // SPO = 0
                                        // FRF = Freescale format
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
                                        // DSS = 16-bit data
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_16;
  SSI0_DR_R = data;                     // load 'data' into transmit FIFO
  SSI0_CR1_R |= SSI_CR1_SSE;            // enable SSI
}

//********DAC_Out*****************
// Send data to MAX5353 12-bit DAC
// inputs:  voltage output (0 to 4095)
// outputs: none
void DAC_Out(uint16_t code){
  while((SSI0_SR_R&SSI_SR_TNF)==0){};// wait until room in FIFO
  SSI0_DR_R = code;                  // data out
//  while((SSI0_SR_R&SSI_SR_RNE)==0){};// wait until response
//  return SSI0_DR_R;                  // acknowledge response
}
