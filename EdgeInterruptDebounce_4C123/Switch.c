// Switch.c
// Runs on TMC4C123
// Use GPIO in edge time mode to request interrupts on any
// edge of PF4 and start Timer0B. In Timer0B one-shot
// interrupts, record the state of the switch once it has stopped
// bouncing.
// Daniel Valvano
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

// PF4 connected to a negative logic switch using internal pull-up (trigger on both edges)
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#define PF4                     (*((volatile uint32_t *)0x40025040))
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile static unsigned long Touch;     // true on touch
volatile static unsigned long Release;   // true on release
volatile static unsigned long Last;      // previous
void (*TouchTask)(void);    // user function to be executed on touch
void (*ReleaseTask)(void);  // user function to be executed on release
static void Timer0Arm(void){
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
  TIMER0_TAILR_R = 160000;      // 4) 10ms reload value
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}
static void GPIOArm(void){
  GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
  GPIO_PORTF_IM_R |= 0x10;      // (f) arm interrupt on PF4 *** No IME bit as mentioned in Book ***
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC  
}
// Initialize switch interface on PF4 
// Inputs:  pointer to a function to call on touch (falling edge),
//          pointer to a function to call on release (rising edge)
// Outputs: none 
void Switch_Init(void(*touchtask)(void), void(*releasetask)(void)){
  // **** general initialization ****
  SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port F
  while((SYSCTL_PRGPIO_R & 0x00000020) == 0){};
  GPIO_PORTF_DIR_R &= ~0x10;    // (c) make PF4 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x10;  //     disable alt funct on PF4
  GPIO_PORTF_DEN_R |= 0x10;     //     enable digital I/O on PF4   
  GPIO_PORTF_PCTL_R &= ~0x000F0000; // configure PF4 as GPIO
  GPIO_PORTF_AMSEL_R = 0;       //     disable analog functionality on PF
  GPIO_PORTF_PUR_R |= 0x10;     //     enable weak pull-up on PF4
  GPIO_PORTF_IS_R &= ~0x10;     // (d) PF4 is edge-sensitive
  GPIO_PORTF_IBE_R |= 0x10;     //     PF4 is both edges
  GPIOArm();

  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  TouchTask = touchtask;           // user function 
  ReleaseTask = releasetask;       // user function 
  Touch = 0;                       // allow time to finish activating
  Release = 0;
  Last = PF4;                      // initial switch state
 }
// Interrupt on rising or falling edge of PF4 (CCP0)
void GPIOPortF_Handler(void){
  GPIO_PORTF_IM_R &= ~0x10;     // disarm interrupt on PF4 
  if(Last){    // 0x10 means it was previously released
    Touch = 1;       // touch occurred
    (*TouchTask)();  // execute user task
  }
  else{
    Release = 1;       // release occurred
    (*ReleaseTask)();  // execute user task
  }
  Timer0Arm(); // start one shot
}
// Interrupt 10 ms after rising edge of PF4
void Timer0A_Handler(void){
  TIMER0_IMR_R = 0x00000000;    // disarm timeout interrupt
  Last = PF4;  // switch state
  GPIOArm();   // start GPIO
}

// Wait for switch to be pressed 
// There will be minimum time delay from touch to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitPress(void){
  while(Touch==0){}; // wait for press
  Touch = 0;  // set up for next time
}

// Wait for switch to be released 
// There will be minimum time delay from release to when this function returns
// Inputs:  none
// Outputs: none 
void Switch_WaitRelease(void){
  while(Release==0){}; // wait
  Release = 0; // set up for next time
}

// Return current value of the switch 
// Repeated calls to this function may bounce during touch and release events
// If you need to wait for the switch, use WaitPress or WaitRelease
// Inputs:  none
// Outputs: false if switch currently pressed, true if released 
unsigned long Switch_Input(void){
  return PF4;
}
