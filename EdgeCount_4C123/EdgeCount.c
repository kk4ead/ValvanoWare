// EdgeCount.c
// Runs on LM4F120/TM4C123
// Use Timer0A in edge count mode to count the rising edge on PB6.
// Jonathan Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Example 6.xx, Program 6.xx

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

// external signal connected to PB6 (T0CCP0) (count on rising edge)

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

/* In Edge-Count mode, the timer is configured as a 24-bit or 48-bit up- or up- or down-counter including
the optional prescaler with the upper count value stored in the GPTM Timer n Prescale (GPTMTnPR)
register and the lower bits in the GPTMTnR register. In this mode, the timer is capable of capturing
three types of events: rising edge, falling edge, or both. To place the timer in Edge-Count mode,
the TnCMR bit of the GPTMTnMR register must be cleared. The type of edge that the timer counts
is determined by the TnEVENT fields of the GPTMCTL register. During initialization in down-count
mode, the GPTMTnMATCHR and GPTMTnPMR registers are configured so that the difference
between the value in the GPTMTnILR and GPTMTnPR registers and the GPTMTnMATCHR and
GPTMTnPMR registers equals the number of edge events that must be counted. In up-count mode,
the timer counts from 0x0 to the value in the GPTMTnMATCHR and GPTMTnPMR registers. Note
that when executing an up-count, that the value of GPTMTnPR and GPTMTnILR must be greater
than the value of GPTMTnPMR and GPTMTnMATCHR. Table 11-8 on page 710 shows the values
that are loaded into the timer registers when the timer is enabled.
*/
//------------EdgeCount_Init------------
// Initialize Timer0A in edge count mode to increment Timer0A
// on the rising edge of PB6 (T0CCP0).
// Input: none
// Output: none
void EdgeCount_Init(void){
  SYSCTL_RCGCTIMER_R |= 0x01;  // activate Timer 0
  SYSCTL_RCGCGPIO_R |= 0x02;   // activate Port B
                               // allow time to finish activating
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};
  GPIO_PORTB_DIR_R &= ~0x40;   // make PB6 in
  GPIO_PORTB_AFSEL_R |= 0x40;  // enable alt funct on PB6
  GPIO_PORTB_DEN_R |= 0x40;    // enable digital I/O on PB6
                               // configure PB6 as T0CCP0
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xF0FFFFFF)+0x07000000;
  GPIO_PORTB_AMSEL_R &= ~0x40; // disable analog functionality on PB6
  TIMER0_CTL_R &= ~0x00000001; // disable timer0A during setup
  TIMER0_CFG_R = 0x00000004;   // configure for 16-bit timer mode
   // configure for capture mode, edge-count, up-count settings
   // In the GPTM Timer Mode (GPTMTnMR) register, write the TnCDIR field to 0x1,
   // the TnAMS field to 0x0, the TnCMR field to 0x0, and the TnMR field to 0x3.
  TIMER0_TAMR_R = 0x13;        // edge count, count up
  TIMER0_CTL_R &= ~(0xC);      // configure for rising edge event
  TIMER0_TAILR_R = 0xFFFF;
  TIMER0_TAPR_R = 0xFF;        // counts up to 0xFFFFFF (16,777,215)
  TIMER0_TAMATCHR_R = 0xFFFF;
  TIMER0_TAPMR_R = 0xFF;
  TIMER0_CTL_R |= 0x00000001;  // enable timer0A 16-b, positive edge-count, up-count
}

//------------EdgeCount_Result------------
// Count the number of rising edges since initialization.
// Input: none
// Output: 24-bit count of edges
uint32_t EdgeCount_Result(void){
  return TIMER0_TAR_R;
}
