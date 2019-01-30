// FixedScheduler.c
// Runs on LM4F120
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano
// May 3, 2015

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015

   Program 5.12, section 5.7

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

// logic analyzer connected to PB2,PB1,PB0 for profiling
#include <stdint.h>
#include "PLL.h"
#include "../inc/tm4c123gh6pm.h"
#define PB3        (*((volatile uint32_t *)0x40005020))
#define PB2        (*((volatile uint32_t *)0x40005010))
#define PB1        (*((volatile uint32_t *)0x40005008))
#define PB0        (*((volatile uint32_t *)0x40005004))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void OS_Launch(void);    
void OS_Suspend(void){
  NVIC_INT_CTRL_R = 0x10000000;    // trigger PendSV
}
void WaitForInterrupt(void);  // low power mode
volatile uint32_t Counts = 0;
//******************FSM**************************
struct State{
  uint32_t Out;                 // Output to Port xxx
  const struct State *Next[4];  // Next state if input=0,1,2,3
};
typedef const struct State StateType;
typedef StateType *	StatePtr;

#define SA &fsm[0]
#define SB &fsm[1]
#define SC &fsm[2]
#define SD &fsm[3]
#define SE &fsm[4]
#define SF &fsm[5]
StateType fsm[6]={
	{0x01,{SB,SC,SD,SE}},  // SA,SB alternate toggle
	{0x02,{SA,SC,SD,SE}},  // SB
	{0x03,{SA,SC,SD,SE}},  // SC both on
	{0x00,{SA,SC,SD,SE}},  // SD both off
	{0x00,{SA,SC,SD,SF}},  // SE,SF together toggle
	{0x03,{SA,SC,SD,SE}}   // SF
};
void Port_Out(uint32_t data){
}
uint32_t Port_In(void){ 
  return 0; 
}
void FSM(void){ StatePtr Pt;   uint8_t in;
  Pt = SA;                 // Initial State
  for(;;) {
    OS_Suspend();          // Runs every 2ms
    PB0 ^= 0x01;           // profile
    Port_Out(Pt->Out);     // Output depends on the current state
    in = Port_In();
    Pt = Pt->Next[in];     // Next state depends on the input 
  }
}

//******************PID**************************
void PID_Init(void){
}
uint8_t PID_In(void){
  return 0;
}
uint8_t PID_Calc(uint8_t speed){
  return speed;
}
void PID_Out(uint8_t speed){
}
void PID(void){ uint8_t speed,power;   
  PID_Init();              // Initialize
  for(;;) {
    OS_Suspend();          // Runs every 1ms
    PB1 ^= 0x02;           // profile
    speed = PID_In();      // read tachometer 
    power = PID_Calc(speed);
    PID_Out(power);        // adjust power to motor 
  }
}
//******************DAS**************************
void DAS_Init(void){
}
uint8_t DAS_In(void){
  return 0;
}
uint8_t DAS_Calc(uint8_t speed){
  return speed;
}
void DAS(void){ uint8_t raw,Result;   
  DAS_Init();            // Initialize
  for(;;) {
    OS_Suspend();        // Runs every 1.5ms
    PB2 ^= 0x04;         // profile
    raw = DAS_In();      // read ADC 
    Result = DAS_Calc(raw); 
  }
}
//******************PAN**************************
void PAN_Init(void){
}
uint8_t PAN_In(void){
  return 0;
}
void PAN_Out(uint8_t speed){
}
void PAN(void){ uint8_t input;   
  PAN_Init();            // Initialize
  for(;;) {
    input = PAN_In();    // front panel input
    if(input){
      PAN_Out(input);    // process
    }
    PB3 ^= 0x08;
  }
}
// each TCB is 100 words, 400 bytes
struct TCB{
  uint32_t *StackPt;       // Stack Pointer
  uint32_t MoreStack[83];  // 396 bytes of stack 
  uint32_t InitialReg[14]; // R4-R11,R0-R3,R12,R14
  uint32_t InitialPC;      // pointer to program to execute
  uint32_t InitialPSR;     // 0x01000000
};
typedef struct TCB TCBType;
TCBType *RunPt;            // thread currently running 
#define TheFSM &sys[0]     // finite state machine
#define ThePID &sys[1]     // proportional-integral-derivative
#define TheDAS &sys[2]     // data acquisition system
#define ThePAN &sys[3]     // front panel
TCBType sys[4]={
  { &sys[0].InitialReg[0],{ 0}, { 0},(uint32_t) FSM, 0x01000000},
  { &sys[1].InitialReg[0],{ 0}, { 0},(uint32_t) PID, 0x01000000},
  { &sys[2].InitialReg[0],{ 0}, { 0},(uint32_t) DAS, 0x01000000},
  { &sys[3].InitialReg[0],{ 0}, { 0},(uint32_t) PAN, 0x01000000}
};
struct Node{
  struct Node *Next;        // circular linked list
  TCBType *ThreadPt;        // which thread to run
  uint32_t TimeSlice; // how long to run it
};
typedef struct Node NodeType;
NodeType *NodePt;
NodeType Schedule[22]={ 
{ &Schedule[1], ThePID, 300*80}, // interval     0,  300
{ &Schedule[2], TheFSM, 100*80}, // interval   300,  400
{ &Schedule[3], TheDAS,  50*80}, // interval   400,  450
{ &Schedule[4], ThePAN, 550*80}, // interval   450, 1000
{ &Schedule[5], ThePID, 300*80}, // interval  1000, 1300
{ &Schedule[6], ThePAN, 600*80}, // interval  1300, 1900
{ &Schedule[7], TheDAS,  50*80}, // interval  1900, 1950
{ &Schedule[8], ThePAN,  50*80}, // interval  1950, 2000
{ &Schedule[9], ThePID, 300*80}, // interval  2000, 2300
{ &Schedule[10],TheFSM, 100*80}, // interval  2300, 2400
{ &Schedule[11],ThePAN, 600*80}, // interval  2400, 3000
{ &Schedule[12],ThePID, 300*80}, // interval  3000, 3300
{ &Schedule[13],ThePAN, 100*80}, // interval  3300, 3400
{ &Schedule[14],TheDAS,  50*80}, // interval  3400, 3450
{ &Schedule[15],ThePAN, 550*80}, // interval  3450, 4000
{ &Schedule[16],ThePID, 300*80}, // interval  4000, 4300
{ &Schedule[17],TheFSM, 100*80}, // interval  4300, 4400
{ &Schedule[18],ThePAN, 500*80}, // interval  4400, 4900
{ &Schedule[19],TheDAS,  50*80}, // interval  4900, 4950
{ &Schedule[20],ThePAN,  50*80}, // interval  4950, 5000
{ &Schedule[21],ThePID, 300*80}, // interval  5000, 5300
{ &Schedule[0], ThePAN, 700*80}  // interval  5300, 6000
};

int main(void){
  PLL_Init(Bus80MHz);         // bus clock at 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x02;  // activate port B
  Counts = 0;
  GPIO_PORTB_DIR_R |= 0x0F;   // make PB3-0 output 
  GPIO_PORTB_AFSEL_R &= ~0x0F;// disable alt funct on PB3-0
  GPIO_PORTB_DEN_R |= 0x0F;   // enable digital I/O on PB3-0
                              // configure PB3-0 as GPIO
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF0000)+0x00000000;
  GPIO_PORTB_AMSEL_R &= ~0x0F;    // disable analog functionality on PB3-0
  NodePt = &Schedule[0];      // Specify first thread to run 
  RunPt = NodePt->ThreadPt;
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = NodePt->TimeSlice-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
                              // enable SysTick with core clock and interrupts
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC+NVIC_ST_CTRL_INTEN;

  EnableInterrupts();
  OS_Launch();
  while(1){                   // interrupts every 1ms, 500 Hz flash
  }
}


