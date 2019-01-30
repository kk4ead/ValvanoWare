// PWMDual.c
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate two square waves with 50% duty cycle
// and 180 degrees out of phase.
// Daniel Valvano
// September 10, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
  Program 6.7, section 6.3.2

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
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

#define PWM_ENABLE_PWM1EN       0x00000002  // PWM1 Output Enable
#define PWM_ENABLE_PWM0EN       0x00000001  // PWM0 Output Enable
#define PWM_0_CTL_MODE          0x00000002  // Counter Mode
#define PWM_0_CTL_ENABLE        0x00000001  // PWM Block Enable
#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define PWM_0_GENA_ACTZERO_ONE  0x00000003  // Set the output signal to 1
#define PWM_0_GENB_ACTCMPAD_ZERO                                              \
                                0x00000080  // Set the output signal to 0
#define PWM_0_GENB_ACTCMPAU_ONE 0x00000030  // Set the output signal to 1
#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor
#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor
#define SYSCTL_RCC_PWMDIV_64    0x000A0000  // PWM clock /64
#define SYSCTL_RCGC0_PWM0       0x00100000  // PWM Clock Gating Control
#define SYSCTL_RCGC2_GPIOB      0x00000002  // Port B Clock Gating Control

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/64 
//                = 3.2 MHz/64 = 50 kHz (in this example)
void PWM0Dual_Init(uint16_t period){
  volatile uint32_t delay;
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  delay = SYSCTL_RCGCGPIO_R;            // allow time to finish activating
  GPIO_PORTB_AFSEL_R |= 0xC0;           // enable alt funct on PB7-6
  GPIO_PORTB_PCTL_R &= ~0xFF000000;     // configure PB7-6 as PWM0
  GPIO_PORTB_PCTL_R |= 0x44000000;
  GPIO_PORTB_AMSEL_R &= ~0xC0;          // disable analog functionality on PB7-6
  GPIO_PORTB_DEN_R |= 0xC0;             // enable digital I/O on PB7-6
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_64; //    configure for /64 divider
  PWM0_0_CTL_R = 0;                     // 4) disable PWM while initializing
  //PWM0, Generator A (PWM0/PB6) goes to 0 when count==reload and 1 when count==0
  PWM0_0_GENA_R = (PWM_0_GENA_ACTLOAD_ZERO|PWM_0_GENA_ACTZERO_ONE);
  //PWM0, Generator B (PWM1/PB7) goes to 0 when count==CMPA counting down and 1 when count==CMPA counting up
  PWM0_0_GENB_R = (PWM_0_GENB_ACTCMPAD_ZERO|PWM_0_GENB_ACTCMPAU_ONE);
  PWM0_0_LOAD_R = (period - 1)/2;       // 5) count from zero to this number and back to zero in (period - 1) cycles
  PWM0_0_CMPA_R = (period - 1)/4;       // 6) count value when PWM1/PB7 toggles
                                        // 7) start PWM0 in Count Up/Down mode
  PWM0_0_CTL_R |= (PWM_0_CTL_MODE|PWM_0_CTL_ENABLE);
                                        // enable PWM1-0
  PWM0_ENABLE_R |= (PWM_ENABLE_PWM1EN|PWM_ENABLE_PWM0EN);
}
// change period
// period is 16-bit number of PWM clock cycles in one period (3<=period)
void PWM0Dual_Period(uint16_t period){
  PWM0_0_LOAD_R = (period - 1)/2;       // 5) count from zero to this number and back to zero in (period - 1) cycles
  PWM0_0_CMPA_R = (period - 1)/4;       // 6) count value when PWM1/PB7 toggles
}
