// Timer0.c
// Runs on LM4F120/TM4C123
// Use Timer0 in 32-bit periodic mode to request interrupts at a
// particular period.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013
   Program 7.5, example 7.6

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

#define NVIC_EN0_INT19          0x00080000  // Interrupt 19 enable
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))  // IRQ 0 to 31 Set Enable Register
#define NVIC_PRI4_R             (*((volatile unsigned long *)0xE000E410))  // IRQ 16 to 19 Priority Register
#define TIMER0_CFG_R            (*((volatile unsigned long *)0x40030000))
#define TIMER0_TAMR_R           (*((volatile unsigned long *)0x40030004))
#define TIMER0_CTL_R            (*((volatile unsigned long *)0x4003000C))
#define TIMER0_IMR_R            (*((volatile unsigned long *)0x40030018))
#define TIMER0_MIS_R            (*((volatile unsigned long *)0x40030020))
#define TIMER0_ICR_R            (*((volatile unsigned long *)0x40030024))
#define TIMER0_TAILR_R          (*((volatile unsigned long *)0x40030028))
#define TIMER0_TAPR_R           (*((volatile unsigned long *)0x40030038))
#define TIMER0_TAR_R            (*((volatile unsigned long *)0x40030048))
#define TIMER_CFG_32_BIT_TIMER  0x00000000  // 32-bit timer configuration
#define TIMER_TAMR_TACDIR       0x00000010  // GPTM Timer A Count Direction
#define TIMER_TAMR_TAMR_PERIOD  0x00000002  // Periodic Timer mode
#define TIMER_CTL_TAEN          0x00000001  // GPTM TimerA Enable
#define TIMER_IMR_TATOIM        0x00000001  // GPTM TimerA Time-Out Interrupt
                                            // Mask
#define TIMER_ICR_TATOCINT      0x00000001  // GPTM TimerA Time-Out Raw
                                            // Interrupt
#define TIMER_TAILR_M           0xFFFFFFFF  // GPTM Timer A Interval Load
                                            // Register
#define SYSCTL_RCGC1_R          (*((volatile unsigned long *)0x400FE104))
#define SYSCTL_RCGC1_TIMER0     0x00010000  // timer 0 Clock Gating Control

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void (*PeriodicTask)(void);   // user function

// ***************** Timer0_Init ****************
// Activate Timer0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer0_Init(void(*task)(void), unsigned long period){long sr;
  sr = StartCritical(); 
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER0; // 0) activate timer0
  PeriodicTask = task;             // user function
  TIMER0_CTL_R &= ~TIMER_CTL_TAEN; // 1) disable timer0A during setup
                                   // 2) configure for 32-bit timer mode
  TIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;
                                   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
  TIMER0_TAILR_R = period-1;       // 4) reload value
                                   // 5) clear timer0A timeout flag
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;
  TIMER0_IMR_R |= TIMER_IMR_TATOIM;// 6) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x40000000; // 7) priority 2
  NVIC_EN0_R = NVIC_EN0_INT19;     // 8) enable interrupt 19 in NVIC
  TIMER0_CTL_R |= TIMER_CTL_TAEN;  // 9) enable timer0A
  EndCritical(sr);
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
  (*PeriodicTask)();                // execute user task
}
