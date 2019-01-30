// FIFOTestMain.c
// Runs on any LM4F or TM4C microcontroller
// Test the functions provided in FIFO.c in two ways.  First, single-
// step through the first four tests of the program while watching the
// variables "letter" and "result" in the debugger.  Verify that they
// have the expected values after each line.  This test should show
// that the functions correctly implement a FIFO in the absence of
// complicating interrupts.  For the fifth test, let the program run
// for an extended period of time.  An interrupt will periodically
// fill the FIFO, and the main() function will continually empty the
// FIFO, checking that the actual received value matches the expected
// value.  This test is meant to show that interrupts at any point in
// the execution of the get() function will never cause the FIFO to
// lose data or return out of order.  Avoid single-stepping,
// especially in the main (foreground) function, because interrupts
// may continue between each step.  This may result in the FIFO
// erroneously overflowing, since it is not being emptied in the
// foreground.  To see the results of the test, look at the  LED
// (on PF3) or watch the global variables particularly "NumFullError",
// "EnteredCount", "Errors", and "LineHistogram".  The LED will be lit
// if "Errors" is non-zero, and the "LineHistogram" will give a record
// of which line of the get() function was interrupted.  The last
// index of the histogram represents the number of times that the
// interrupt missed the get() function.
// Daniel Valvano
// May 3, 2015

// Connect PF3,PF2,PF1 to logic analyzer
// PF3 high during timer ISR
// PF2 high during non critical part of main
// PF1 high during critical part of main (RxFifo_Get)


#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "PLL.h"
#include "Fifo.h"
#include "Timer0A.h"


#ifdef __TI_COMPILER_VERSION__
  //Code Composer Studio Code
  unsigned long Who(void){
  __asm (  " LDR R0, [SP,#48]\n"); // R0 = return address from interrupt
}

#else
  //Keil uVision Code
  __asm unsigned long
  Who(void){
    LDR R0, [SP,#48]               ; R0 = return address from interrupt
    bx  LR
  }

#endif
#define TX2FIFOSIZE             32
#define RX2FIFOSIZE             27
#define HISTOGRAMSIZE           32          // > 2*# instructions in get()
#define INTPERIOD              1000         // interrupt period (80MHz cycles)
#define INTVARIATION            300         // maximum interrupt period variation from "INTPERIOD" (50MHz cycles)
AddIndexFifo(Tx2, TX2FIFOSIZE, char, 1, 0)
AddPointerFifo(Rx2, RX2FIFOSIZE, char, 1, 0)

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// basic test variables
volatile int result;
char letter;
// background variables
char BackData;               // data which is put
char BackCounter;            // number of puts per interrupt
uint32_t EnteredCount;  // number of times Put has interrupted Get
                             // record of lines where interrupt occurred
uint32_t LineHistogram[HISTOGRAMSIZE];
uint32_t LineHistogramAddress[HISTOGRAMSIZE];
// foreground variables
char ForeExpected;           // expected data
char ForeActual;             // actual data
uint32_t Errors;       // number of out-of-sequence points
uint32_t EnterGet;     // true if program has entered Get

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void UserTask(void){
  uint32_t returnaddress;
  unsigned char i;
  static char periodShift = 1;
  PF3 = 0x08;               // debuging profile
  if(INTVARIATION != 0){            // set time of next interrupt if needed
    TIMER0_TAILR_R = TIMER0_TAILR_R + periodShift;
                                    // check upper bound
    if(((TIMER0_TAILR_R + periodShift) > (INTPERIOD + INTVARIATION - 1)) ||
                                    // check lower bound
     ((TIMER0_TAILR_R + periodShift) < (INTPERIOD - INTVARIATION - 1))){
      periodShift = -periodShift; // start counting in other direction
    }
  }
  if(EnterGet){        // Put has interrupted Get
    EnteredCount = EnteredCount + 1;
  }
  for(i=0; i<=BackCounter; i=i+1){
    if(RxFifo_Put(BackData)){
      BackData = BackData + 1; // sequence is 0,1,2,3,...,254,255,0,1,...
    }                          // ignore full FIFO
  }
  BackCounter = BackCounter + 1;
  if(BackCounter == 5){
    BackCounter = 0;   // 0 to 4
  }
  returnaddress = Who();
  if((returnaddress < (uint32_t)&RxFifo_Get) || (returnaddress >= (uint32_t)&RxFifo_Size)){
    returnaddress = (uint32_t)&RxFifo_Size;
  }
  LineHistogram[(returnaddress - (uint32_t)&RxFifo_Get)/2]++;
  PF3 = 0x00;
}


//debug code
int main(void){ 
  int i;
  PLL_Init(Bus80MHz);              // bus clock at 80 MHz
  SYSCTL_RCGCGPIO_R |= 0x20;       // activate port F
  while((SYSCTL_PRGPIO_R&0x20)==0){};
  GPIO_PORTF_DIR_R |= 0x0E;        // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0x0E;     // disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x0E;        // enable digital I/O on PF3-1
                                   // configure PF3-1 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF000F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PF
  BackData = 0;                    // allow time to finish activating
  BackCounter = 0;
  EnteredCount = 0;
  ForeExpected = 0;
  Errors = 0;
  EnterGet = 0;

  Timer0A_Init(&UserTask, INTPERIOD);  // start value to count down from
    // *************** Test #5: test interrupt vulnerability ************
  RxFifo_Init();
  for(i=0; i<HISTOGRAMSIZE; i=i+1){ uint32_t returnaddress;
    LineHistogram[i] = 0;
    returnaddress = ((uint32_t)&RxFifo_Get + 2*i)&0xFFFFFFFE;
    LineHistogramAddress[i] = returnaddress;   // possible places in Get that could be interrupted
  }
  EnableInterrupts();

  while(1){
    do{
      PF1 = 2;       // profile of main program
      EnterGet = 1;
      i = RxFifo_Get(&ForeActual);  // i = 0 (FIFOFAIL) if error
      EnterGet = 0;
      PF1 = 0; 
    }
    while(!i);
    PF2 = 4;       // profile of main program

    if(ForeExpected != ForeActual){
      Errors = Errors + 1;           // critical section found
      ForeExpected = ForeActual + 1; // resych to lost/bad data
    }
    else{
      ForeExpected = ForeExpected + 1;// sequence is 0,1,2,3,...,254,255,0,1,...
    }
    PF2 = 0; 
  }
}
