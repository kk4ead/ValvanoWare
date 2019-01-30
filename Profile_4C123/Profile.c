// Profile.c
// Runs on LM4F120/TM4C123
// Use SysTick and Timer0A in periodic mode to demonstrate profiling
// PA5 will toggle when the software is running in the foreground 
// PA4 will pulse high then low when executing the SysTick ISR
// PA3 will pulse high then low when executing the Timer ISR
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
  Program 9.13

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

// oscilloscope connected to PA5,PA4,PA3 for profiling

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
 
#define PA5   (*((volatile uint32_t *)0x40004080))
#define PA4   (*((volatile uint32_t *)0x40004040))
#define PA3   (*((volatile uint32_t *)0x40004020))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// The two #define statements SYSDIV and LSB 0 
// initialize the PLL to the desired frequency.
#define SYSDIV 3
#define LSB 1
// bus frequency is 400MHz/(2*SYSDIV+1+LSB) = 400MHz/(2*3+1+1) = 50 MHz

// configure the system to get its clock from the PLL
void PLL_Init(void){
  // 1) configure the system to use RCC2 for advanced features
  //    such as 400 MHz PLL and non-integer System Clock Divisor
  SYSCTL_RCC2_R |= SYSCTL_RCC2_USERCC2;
  // 2) bypass PLL while initializing
  SYSCTL_RCC2_R |= SYSCTL_RCC2_BYPASS2;
  // 3) select the crystal value and oscillator source
  SYSCTL_RCC_R &= ~SYSCTL_RCC_XTAL_M;   // clear XTAL field
  SYSCTL_RCC_R += SYSCTL_RCC_XTAL_16MHZ;// configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_OSCSRC2_M;// clear oscillator source field
  SYSCTL_RCC2_R += SYSCTL_RCC2_OSCSRC2_MO;// configure for main oscillator source
  // 4) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_PWRDN2;
  // 5) use 400 MHz PLL
  SYSCTL_RCC2_R |= SYSCTL_RCC2_DIV400;
  // 6) set the desired system divider and the system divider least significant bit
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_SYSDIV2_M;  // clear system clock divider field
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_SYSDIV2LSB; // clear bit SYSDIV2LSB

// set SYSDIV2 and SYSDIV2LSB fields
  SYSCTL_RCC2_R += (SYSDIV<<23)|(LSB<<22);  // divide by (2*SYSDIV+1+LSB)

  // 7) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&SYSCTL_RIS_PLLLRIS)==0){};
  // 8) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~SYSCTL_RCC2_BYPASS2;
}

// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 20ns (assuming 50 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
volatile uint32_t Counts;
void SysTick_Init(uint32_t period){
  Counts = 0;
  NVIC_ST_CTRL_R = 0;            // disable SysTick during setup
  NVIC_ST_RELOAD_R = period - 1; // reload value
  NVIC_ST_CURRENT_R = 0;         // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; //priority 2                
  NVIC_ST_CTRL_R = 0x00000007;   // enable with core clock and interrupts
} 

// ***************** Timer0A_Init ****************
// Activate Timer0A interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in usec
// Outputs: none
void Timer0A_Init(unsigned short period){ volatile uint32_t delay;
  SYSCTL_RCGCTIMER_R |= 0x01;      // 0) activate timer0
  delay = SYSCTL_RCGCTIMER_R;      // allow time to finish activating
  TIMER0_CTL_R &= ~0x00000001;     // 1) disable timer0A during setup
  TIMER0_CFG_R = 0x00000004;       // 2) configure for 16-bit timer mode
  TIMER0_TAMR_R = 0x00000002;      // 3) configure for periodic mode
  TIMER0_TAILR_R = period - 1;     // 4) reload value
  TIMER0_TAPR_R = 49;              // 5) 1us timer0A
  TIMER0_ICR_R = 0x00000001;       // 6) clear timer0A timeout flag
  TIMER0_IMR_R |= 0x00000001;      // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x60000000; // 8) priority 3
  NVIC_EN0_R = 1<<19;              // 9) enable interrupt 19 in NVIC
  TIMER0_CTL_R |= 0x00000001;      // 10) enable timer0A
}

void Timer0A_Handler(void){
  PA3 = 0x08;
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
  PA3 = 0;
}
void SysTick_Handler(void){
  PA4 = 0x10;
  Counts = Counts + 1; 
  PA4 = 0;
}
int main(void){ 
  DisableInterrupts();
  PLL_Init();                // configure for 50 MHz clock
  SYSCTL_RCGCGPIO_R |= 0x01;   // 1) activate clock for Port A 
  while((SYSCTL_PRGPIO_R&0x01) == 0){};
  GPIO_PORTA_AMSEL_R &= ~0x38;    // disable analog function
  GPIO_PORTA_PCTL_R &= ~0x00FFF000; // GPIO
  GPIO_PORTA_DIR_R |= 0x38;  // make PA5-3 outputs
  GPIO_PORTA_AFSEL_R &= ~0x38;// disable alt func on PA5-3
  GPIO_PORTA_DEN_R |= 0x38;  // enable digital I/O on PA5-3
                             // configure PA5-3 as GPIO
  Timer0A_Init(5);           // 200 kHz
  SysTick_Init(304);         // 164 kHz
  EnableInterrupts();
  while(1){
    PA5 = PA5^0x20;  
  }
}
