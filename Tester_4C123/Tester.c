// Tester.c
// Runs on LM4F120/TM4C123
// Interface with a custom Booster Pack module to test a LaunchPad.
// After the program is downloaded and the Booster Pack is attached,
// the microcontroller will perform a self-diagnostic to check for
// the most common hardware failures and output the results to
// UART0.  Experience has indicated that experimentation and faulty
// prototype circuit assembly can break the microcontroller's
// digital and analog GPIO pins.  This program tests that each
// digital pin can function as a GPIO digital input and output, and
// it verifies that the analog inputs can measure an analog voltage.
// It assumes that any alternate functions work if their associated
// GPIO pins work.
// Daniel Valvano
// September 12, 2013

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers"
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2013

   "Embedded Systems: Real Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2013

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

// U0Rx (VCP receive) connected to PA0
// U0Tx (VCP transmit) connected to PA1
// All other GPIO pins available on the LaunchPad connected together with
// 10k and 220 ohm resistors such that no two pins have less than 440 ohms
// of resistance between them (NOTE: except for PB7 and PD1 and PB6 and BD0,
// which are connected together by default with zero-ohm R9 and R10 on the
// LaunchPad.)
// See the schematic at:
// http://users.ece.utexas.edu/~valvano/arm/tester/TesterSchematicDesign.pdf

#include "ADCSWTrigger.h"
#include "PLL.h"
#include "Timer0.h"// this version runs in 32-bit timer mode; it uses all of Timer0
#include "UART2.h" // file called "UART2.h" because it is the second, more advanced UART example; hardware UART0 is used

#define GPIO_PORTA              (*((volatile unsigned long *)0x400043F0)) // PA7-2
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
#define GPIO_PORTB              (*((volatile unsigned long *)0x400053FC)) // PB7-0
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTC              (*((volatile unsigned long *)0x400063C0)) // PC7-4
#define GPIO_PORTC_DIR_R        (*((volatile unsigned long *)0x40006400))
#define GPIO_PORTC_AFSEL_R      (*((volatile unsigned long *)0x40006420))
#define GPIO_PORTC_DEN_R        (*((volatile unsigned long *)0x4000651C))
#define GPIO_PORTC_AMSEL_R      (*((volatile unsigned long *)0x40006528))
#define GPIO_PORTC_PCTL_R       (*((volatile unsigned long *)0x4000652C))
#define GPIO_PORTD              (*((volatile unsigned long *)0x4000733C)) // PD7-6 and PD3-0
#define GPIO_PORTD_DIR_R        (*((volatile unsigned long *)0x40007400))
#define GPIO_PORTD_AFSEL_R      (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_DEN_R        (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_LOCK_R       (*((volatile unsigned long *)0x40007520))
#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define GPIO_PORTD_CR_R         (*((volatile unsigned long *)0x40007524))
#define GPIO_PORTD_AMSEL_R      (*((volatile unsigned long *)0x40007528))
#define GPIO_PORTD_PCTL_R       (*((volatile unsigned long *)0x4000752C))
#define GPIO_PORTE              (*((volatile unsigned long *)0x400240FC)) // PE5-0
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define GPIO_PORTF              (*((volatile unsigned long *)0x4002507C)) // PF4-0
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define PF1       (*((volatile unsigned long *)0x40025008))
#define PF2       (*((volatile unsigned long *)0x40025010))
#define PF3       (*((volatile unsigned long *)0x40025020))
#define LEDS      (*((volatile unsigned long *)0x40025038))
#define SWITCHES  (*((volatile unsigned long *)0x40025044))
#define SW1PRESS  0x01        // on the left side of the LaunchPad board (value of SWITCHES when only SW1 pressed)
#define SW2PRESS  0x10        // on the right side of the LaunchPad board (value of SWITCHES when only SW2 pressed)
#define NOSWPRESS 0x11        // value of SWITCHES when no switches pressed
#define BOTHSWPRESS 0x00      // value of SWITCHES when both switches pressed
#define RED       0x02
#define BLUE      0x04
#define GREEN     0x08
#define WHEELSIZE 8           // must be an integer multiple of 2
                              //    red, yellow,    green, light blue, blue, purple,   white,          dark
const long COLORWHEEL[WHEELSIZE] = {RED, RED+GREEN, GREEN, GREEN+BLUE, BLUE, BLUE+RED, RED+GREEN+BLUE, 0};
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOF      0x00000020  // port F Clock Gating Control
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control
#define SYSCTL_RCGC2_GPIOD      0x00000008  // port D Clock Gating Control
#define SYSCTL_RCGC2_GPIOC      0x00000004  // port C Clock Gating Control
#define SYSCTL_RCGC2_GPIOB      0x00000002  // port B Clock Gating Control
#define SYSCTL_RCGC2_GPIOA      0x00000001  // port A Clock Gating Control
#define MAXDELAY                1000 // number of loops before giving up
unsigned short printLines = 0;       // number of lines of error messages printed to UART
#define MAXLINES                20   // maximum number of lines of error messages printed to UART
                                     // putty can neatly handle many lines of text,
                                     // but HyperTerminal has a small buffer and
                                     // cannot scroll back through previous messages
                                     // unless it is set to logging mode beforehand

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

//------------digitaltest------------
// Internal function used to test a GPIO port's digital pins.  It
// sets one pin to output high and all of the other pins to input
// and verifies that the input pins all report high.  Then, it
// clears the output pin and verifies that the input pins all
// report low.  A pin's output is broken if it is stuck
// permanently high or low, and a pin's input is broken if it
// reports a different value than the other input pins. Currently,
// this function makes no attempt to match the pattern of observed
// errors with the most likely explanation.  Instead, if a test
// does not go as expected, it attempts to output an error message
// to the UART.
// Input: firstPinMask    bit mask of the first pin to be tested (ex. 0x10 for PC)
//        lastPinMask     bit mask of the last pin to be tested (ex. 0x80 for PC)
//        allPinMask      bit mask of all pins to be tested, including the first and last pins (ex. 0xF0 for PC)
//        firstPinNumber  bit number of first pin to be tested (ex. 4 for PC)
//        portLetter      letter of port to be tested (ex. 'C' for PC)
//        dataRegPtr      pointer to the data register of the port to be tested (ex. &GPIO_PORTC for PC)
//        directionRegPtr pointer to the direction register of the port to be tested (ex. &GPIO_PORTC_DIR_R for PC)
// Output: char containing bit mask of possibly broken pins (ex. 0x30 if PC5-4 is broken)
char digitaltest(unsigned long firstPinMask,
                 unsigned long lastPinMask,
                 unsigned long allPinMask,
                 unsigned long firstPinNumber,
                 unsigned char portLetter,
                 volatile unsigned long *dataRegPtr,
                 volatile unsigned long *directionRegPtr){
  unsigned long outmask, inmask, bit, obit, delay;
  char errormask = 0;
  outmask = firstPinMask;      // bit mask of digital output being tested
  obit = firstPinNumber;       // bit number of digital output being tested
  while(outmask <= lastPinMask){
                               // make Pn[outmask] out and other pins in
    *directionRegPtr = outmask&allPinMask;
    *dataRegPtr = outmask;     // Pn[outmask] = 1
    delay = 0;
    while(((*dataRegPtr&allPinMask) != allPinMask) && (delay < MAXDELAY)){
      delay = delay + 1;       // wait until Pn[outmask] goes high
    }
    if((*dataRegPtr&allPinMask) != allPinMask){
      errormask |= outmask;    // Pn has an error on at least one pin
      inmask = firstPinMask;   // bit mask of digital input being tested
      bit = firstPinNumber;    // bit number of digital input being tested
      while(inmask <= lastPinMask){
                               // expect each input to also be high
        if(((*dataRegPtr&inmask) == 0) && (inmask != outmask) && (printLines < MAXLINES)){
          UART_OutString("GPIO output ");
          UART_OutChar(portLetter);
          UART_OutUDec(obit);
          UART_OutString(" input ");
          UART_OutChar(portLetter);
          UART_OutUDec(bit);
          UART_OutString(" should be high but is low.\r\n");// Pn[inmask] digital input broken
          printLines = printLines + 1;
        }
        inmask = inmask<<1;    // check the next digital input
        bit = bit + 1;
      }
    }
    *dataRegPtr = 0;            // Pn = 0
    delay = 0;
    while(((*dataRegPtr&allPinMask) != 0x00) && (delay < MAXDELAY)){
      delay = delay + 1;       // wait until Pn[outmask] goes low
    }
    if((*dataRegPtr&allPinMask) != 0x00){
      errormask |= outmask;    // Pn has an error on at least one pin
      inmask = firstPinMask;   // bit mask of digital input being tested
      bit = firstPinNumber;    // bit number of digital input being tested
      while(inmask <= lastPinMask){
                               // expect each input to also be high
        if(((*dataRegPtr&inmask) == inmask) && (inmask != outmask) && (printLines < MAXLINES)){
          UART_OutString("GPIO output ");
          UART_OutChar(portLetter);
          UART_OutUDec(obit);
          UART_OutString(" input ");
          UART_OutChar(portLetter);
          UART_OutUDec(bit);
          UART_OutString(" should be low but is high.\r\n");// Pn[inmask] digital input broken
          printLines = printLines + 1;
        }
        inmask = inmask<<1;    // check the next digital input
        bit = bit + 1;
      }
    }
    *directionRegPtr = 0;      // make Pn[outmask] in
    outmask = outmask<<1;      // check the next digital output
    obit = obit + 1;
  }
  return errormask;
}

//------------ColorWheel------------
// Flash the three-color LED on the LaunchPad in a colorful
// pattern dictated by the COLORWHEEL global variable.  Each time
// this function is called, the color increments.  This function
// assumes that PF3-1 are set as digital GPIO outputs.
// Input: none
// Output: none
void ColorWheel(void){
  static int i = 0;
  LEDS = COLORWHEEL[i&(WHEELSIZE-1)];
  i = i + 1;                   // increment the count
}

//------------FlashRed------------
// Flash the red LED on the LaunchPad.  Each time this function
// is called, the LED turns on or off.  This function assumes that
// PF1 is set as digital GPIO output.
// Input: none
// Output: none
void FlashRed(void){
  static int i = 0;
  if((i%2) == 0){
    LEDS = RED;                // red every other call
  } else{
    LEDS = 0;                  // off every other call
  }
  i = i + 1;                   // increment the count
}

//------------FlashYellow------------
// Flash the yellow LED on the LaunchPad.  Each time this function
// is called, the LED turns on or off.  This function assumes that
// PF3 and PF1 are set as digital GPIO outputs.
// Input: none
// Output: none
void FlashYellow(void){
  static int i = 0;
  if((i%2) == 0){
    LEDS = (RED|GREEN);        // yellow every other call
  } else{
    LEDS = 0;                  // off every other call
  }
  i = i + 1;                   // increment the count
}

//------------FlashGreen------------
// Flash the green LED on the LaunchPad.  Each time this function
// is called, the LED turns on or off.  This function assumes that
// PF3 is set as digital GPIO output.
// Input: none
// Output: none
void FlashGreen(void){
  static int i = 0;
  if((i%2) == 0){
    LEDS = GREEN;              // green every other call
  } else{
    LEDS = 0;                  // off every other call
  }
  i = i + 1;                   // increment the count
}

//------------FlashBlue------------
// Flash the blue LED on the LaunchPad.  Each time this function
// is called, the LED turns on or off.  This function assumes that
// PF2 is set as digital GPIO output.
// Input: none
// Output: none
void FlashBlue(void){
  static int i = 0;
  if((i%2) == 0){
    LEDS = BLUE;               // blue every other call
  } else{
    LEDS = 0;                  // off every other call
  }
  i = i + 1;                   // increment the count
}

int main(void){ volatile unsigned long delay;
  unsigned long outmask, inmask, channel;
  unsigned short in, previous;
  // 0) if error is found, the corresponding bit in these variables is set
  unsigned char analogB=0, analogD=0, analogE=0;
  unsigned char digitalA=0, digitalB=0, digitalC=0, digitalD=0, digitalE=0, digitalF=0;
  // 1) activate all GPIO ports
  SYSCTL_RCGC2_R |= (SYSCTL_RCGC2_GPIOA|  // activate port A
                     SYSCTL_RCGC2_GPIOB|  // activate port B
                     SYSCTL_RCGC2_GPIOC|  // activate port C
                     SYSCTL_RCGC2_GPIOD|  // activate port D
                     SYSCTL_RCGC2_GPIOE|  // activate port E
                     SYSCTL_RCGC2_GPIOF); // activate port F
  delay = SYSCTL_RCGC2_R;      // allow time to finish activating
  // 2) set PA7-2 as GPIO digital inputs
  GPIO_PORTA_DIR_R &= ~0xFC;   // make PA7-2 in
  GPIO_PORTA_AFSEL_R &= ~0xFC; // disable alt funct on PA7-2
  GPIO_PORTA_DEN_R |= 0xFC;    // enable digital I/O on PA7-2
                               // configure PA7-2 as GPIO
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0x000000FF)+0x00000000;
  GPIO_PORTA_AMSEL_R = 0;      // disable analog functionality on PA (no analog on PA)
  // 2) set PB7-0 as GPIO digital inputs
  GPIO_PORTB_DIR_R &= ~0xFF;   // make PB7-0 in
  GPIO_PORTB_AFSEL_R &= ~0xFF; // disable alt funct on PB7-0
  GPIO_PORTB_DEN_R |= 0xFF;    // enable digital I/O on PB7-0
                               // configure PB7-0 as GPIO
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x00000000)+0x00000000;
  GPIO_PORTB_AMSEL_R &= ~0x30; // disable analog functionality on PB5-4 (AIN10 and AIN11)
  // 3) set PC7-4 as GPIO digital inputs
  GPIO_PORTC_DIR_R &= ~0xF0;   // make PC7-4 in
  GPIO_PORTC_AFSEL_R &= ~0xF0; // disable alt funct on PC7-4
  GPIO_PORTC_DEN_R |= 0xF0;    // enable digital I/O on PC7-4
                               // configure PC7-4 as GPIO
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0x0000FFFF)+0x00000000;
  GPIO_PORTC_AMSEL_R &= ~0xF0; // disable analog functionality on PC7-4 (C0-, C0+, C1+, and C1-)
  // 4) unlock PD7 (locked by default due to possible NMI functionality of PD7)
                               // unlock GPIO Port D Commit Register
  GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTD_CR_R = 0x80;      // enable commit for PD7
  // 5) set PD7-6 and PD3-0 as GPIO digital inputs
  GPIO_PORTD_DIR_R &= ~0xCF;   // make PD7-6 and PD3-0 in
  GPIO_PORTD_AFSEL_R &= ~0xCF; // disable alt funct on PD7-6 and PD3-0
  GPIO_PORTD_DEN_R |= 0xCF;    // enable digital I/O on PD7-6 and PD3-0
                               // configure PD7-6 and PD3-0 as GPIO
  GPIO_PORTD_PCTL_R = (GPIO_PORTD_PCTL_R&0x00FF0000)+0x00000000;
  GPIO_PORTD_AMSEL_R &= ~0x0F; // disable analog functionality on PD3-0 (AIN4, AIN5, AIN6, and AIN7)
  // 6) set PE5-0 as GPIO digital inputs
  GPIO_PORTE_DIR_R &= ~0x3F;   // make PE5-0 in
  GPIO_PORTE_AFSEL_R &= ~0x3F; // disable alt funct on PE5-0
  GPIO_PORTE_DEN_R |= 0x3F;    // enable digital I/O on PE5-0
                               // configure PE5-0 as GPIO
  GPIO_PORTE_PCTL_R = (GPIO_PORTE_PCTL_R&0xFF000000)+0x00000000;
  GPIO_PORTE_AMSEL_R &= ~0x3F; // disable analog functionality on PE5-0 (AIN8, AIN9, AIN0, AIN1, AIN2, and AIN3)
  // 7) unlock PF0 (locked by default due to possible NMI functionality of PF0)
                               // unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R = 0x01;      // enable commit for PF0
  // 8) set PF4-0 as GPIO digital inputs
  GPIO_PORTF_DIR_R &= ~0x1F;   // make PF4-0 in
  GPIO_PORTF_AFSEL_R &= ~0x1F; // disable alt funct on PF4-0
  GPIO_PORTF_DEN_R |= 0x1F;    // enable digital I/O on PF4-0
                               // configure PF4-0 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF00000)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;      // disable analog functionality on PF (no analog on PF)
  // 9) activate PLL for 50 MHz clock
  PLL_Init();
  // 10) activate UART0 at 115,200 baud rate, 8 data bits, no parity, 1 stop bit, no hardware flow control
  UART_Init();
  UART_OutString("\r\nLM4F120 Test\r\n");
  // 11) test Port A digital
  digitalA = digitaltest(0x04, 0x80, 0xFC, 2, 'A', &GPIO_PORTA, &GPIO_PORTA_DIR_R);
  // 12) test Port B digital
/*  outmask = 0x01;              // bit mask of digital output being tested
  obit = 0;                    // bit number of digital output being tested
  while(outmask <= 0x80){
                               // make PB[outmask] out and other pins in
    GPIO_PORTB_DIR_R = outmask&0xFF;
    GPIO_PORTB = outmask;      // PB[outmask] = 1
    delay = 0;
    while((GPIO_PORTB != 0xFF) && (delay < MAXDELAY)){
      delay = delay + 1;       // wait until PB[outmask] goes high
    }
    if(GPIO_PORTB != 0xFF){
      digitalB |= outmask;     // PB has an error on at least one pin
      inmask = 0x01;           // bit mask of digital input being tested
      bit = 0;                 // bit number of digital input being tested
      while(inmask <= 0x80){   // expect each input to also be high
        if(((GPIO_PORTB&inmask) == 0) && (inmask != outmask)){
          UART_OutString("GPIO output B");
          UART_OutUDec(obit);
          UART_OutString(" input B");
          UART_OutUDec(bit);
          UART_OutString(" should be high but is low.\r\n");// PB[inmask] digital input broken
        }
        inmask = inmask<<1;    // check the next digital input
        bit = bit + 1;
      }
    }
    GPIO_PORTB = 0;            // PB = 0
    delay = 0;
    while((GPIO_PORTB != 0x00) && (delay < MAXDELAY)){
      delay = delay + 1;       // wait until PB[outmask] goes low
    }
    if(GPIO_PORTB != 0x00){
      digitalB |= outmask;     // PB has an error on at least one pin
      inmask = 0x01;           // bit mask of digital input being tested
      bit = 0;                 // bit number of digital input being tested
      while(inmask <= 0x80){   // expect each input to also be high
        if(((GPIO_PORTB&inmask) == inmask) && (inmask != outmask)){
          UART_OutString("GPIO output B");
          UART_OutUDec(obit);
          UART_OutString(" input B");
          UART_OutUDec(bit);
          UART_OutString(" should be low but is high.\r\n");// PB[inmask] digital input broken
        }
        inmask = inmask<<1;    // check the next digital input
        bit = bit + 1;
      }
    }
    GPIO_PORTB_DIR_R = 0;      // make PB[outmask] in
    outmask = outmask<<1;      // check the next digital output
    obit = obit + 1;
  }*/
  digitalB = digitaltest(0x01, 0x80, 0xFF, 0, 'B', &GPIO_PORTB, &GPIO_PORTB_DIR_R);
  // 13) test Port C digital
  digitalC = digitaltest(0x10, 0x80, 0xF0, 4, 'C', &GPIO_PORTC, &GPIO_PORTC_DIR_R);
  // 14) test Port D digital
  digitalD |= digitaltest(0x01, 0x08, 0x0F, 0, 'D', &GPIO_PORTD, &GPIO_PORTD_DIR_R);
  digitalD |= digitaltest(0x40, 0x80, 0xC0, 6, 'D', &GPIO_PORTD, &GPIO_PORTD_DIR_R);
  // 15) test Port E digital
  digitalE = digitaltest(0x01, 0x20, 0x3F, 0, 'E', &GPIO_PORTE, &GPIO_PORTE_DIR_R);
  // 16) test Port F digital
  digitalF = digitaltest(0x01, 0x10, 0x1F, 0, 'F', &GPIO_PORTF, &GPIO_PORTF_DIR_R);
  // 17) test Port B analog (PB4=AIN10 and PB5=AIN11)
  inmask = 0x10;               // bit mask of analog input being tested
  channel = 10;                // channel number of analog input being tested
  while(channel <= 11){
                               // make rest of PB out
    GPIO_PORTB_DIR_R |= (~inmask)&0xFF;
                               // disable alt funct on PB7-0
    GPIO_PORTB_AFSEL_R &= ~0xFF;
                               // enable digital I/O on PB7-0
    GPIO_PORTB_DEN_R |= (~inmask)&0xFF;
                               // disable analog functionality on PB5-4 (AIN11 and AIN10)
    GPIO_PORTB_AMSEL_R &= ~0x30;
    GPIO_PORTB = 0;            // PB = 0
                               // make AIN[channel] analog in
    ADC0_InitSWTriggerSeq3(channel);
                               // short (possibly unnecessary) delay
    for(delay=0; delay<MAXDELAY; delay=delay+1){};
    previous = ADC0_InSeq3();  // take ADC sample
                               // ***Important: This should measure approximately zero.
                               // If a digital output pin is broken high (always +3.3),
                               // this will be non-zero and may cause a false-positive
                               // error.
    outmask = 0x01;            // initial output to set
                               // if this output is broken, go to the next one
                               // if they are all broken, this test will do nothing
    while(((digitalB&outmask) || (outmask == inmask)) && (outmask <= 0x80)){
      outmask = outmask<<1;
    }
    while(outmask <= 0x80){
      GPIO_PORTB |= outmask;   // set the next output pin high
                               // short (possibly unnecessary) delay
      for(delay=0; delay<MAXDELAY; delay=delay+1){};
      in = ADC0_InSeq3();      // take ADC sample
      if(in <= previous){      // basic linearity test; samples should increase
        analogB |= inmask;     // PB[inmask] analog input broken
        if(printLines < MAXLINES){
          UART_OutString("ADC input PB");
          switch(channel){
            case 10: UART_OutChar('4'); break;
            case 11: UART_OutChar('5'); break;
            default: UART_OutChar('?');
          }
          UART_OutString(" (AIN");
          UART_OutUDec(channel);
          UART_OutString(") expected at least ");
          UART_OutUDec((unsigned long)previous);
          UART_OutString(" but measured ");
          UART_OutUDec((unsigned long)previous);
          UART_OutString(".\r\n");
          printLines = printLines + 1;
        }
      }
      previous = in;
      do{
                               // go to the next digital output pin
                               // if this pin is broken, go to the next one
                               // if this pin is the analog input, go to the next one
        outmask = outmask<<1;  // if run out of pins, break out of loop
      } while(((digitalB&outmask) || (outmask == inmask)) && (outmask <= 0x80));
    }
    inmask = inmask<<1;        // check next analog input
    channel = channel + 1;     // check next analog input
  }
  GPIO_PORTB = 0;              // PB = 0
  GPIO_PORTB_DIR_R &= ~0xFF;   // make PB7-0 in
  GPIO_PORTB_AFSEL_R &= ~0xFF; // disable alt funct on PB7-0
  GPIO_PORTB_DEN_R |= 0xFF;    // enable digital I/O on PB7-0
  GPIO_PORTB_AMSEL_R &= ~0x30; // disable analog functionality on PB5-4 (AIN11 and AIN10)
  // 18) test Port D analog (PD3=AIN4, PD2=AIN5, PD1=AIN6, and PD0=AIN7)
  inmask = 0x01;               // bit mask of analog input being tested
  channel = 7;                 // channel number of analog input being tested
  while(channel >= 4){
                               // make PD7-6 and PD3-0 out
    GPIO_PORTD_DIR_R |= (~inmask)&0xCF;
                               // disable alt funct on PD7-6 and PD3-0
    GPIO_PORTD_AFSEL_R &= ~0xCF;
                               // enable digital I/O on PD7-6 and PD3-0
    GPIO_PORTD_DEN_R |= (~inmask)&0xCF;
                               // disable analog functionality on PD3-0 (AIN4, AIN5, AIN6, and AIN7)
    GPIO_PORTD_AMSEL_R &= ~0x0F;
    GPIO_PORTD &= ~0xCF;       // PD = 0
                               // make AIN[channel] analog in
    ADC0_InitSWTriggerSeq3(channel);
                               // short (possibly unnecessary) delay
    for(delay=0; delay<MAXDELAY; delay=delay+1){};
    previous = ADC0_InSeq3();  // take ADC sample
                               // ***Important: This should measure approximately zero.
                               // If a digital output pin is broken high (always +3.3),
                               // this will be non-zero and may cause a false-positive
                               // error.
    outmask = 0x01;            // initial output to set
                               // if this output is broken, go to the next one
                               // if they are all broken, this test will do nothing
    while(((digitalD&outmask) || (outmask == inmask)) && (outmask <= 0x80)){
      outmask = outmask<<1;
      if(outmask == 0x10){
        outmask = 0x40;        // skip PD4 and PD5
      }
    }
    while(outmask <= 0x80){
      GPIO_PORTD |= outmask;   // set the next output pin high
                               // short (possibly unnecessary) delay
      for(delay=0; delay<MAXDELAY; delay=delay+1){};
      in = ADC0_InSeq3();      // take ADC sample
      if(in <= previous){      // basic linearity test; samples should increase
        analogD |= inmask;     // PD[inmask] analog input broken
        if(printLines < MAXLINES){
          UART_OutString("ADC input PD");
          switch(channel){
            case 4: UART_OutChar('3'); break;
            case 5: UART_OutChar('2'); break;
            case 6: UART_OutChar('1'); break;
            case 7: UART_OutChar('0'); break;
            default: UART_OutChar('?');
          }
          UART_OutString(" (AIN");
          UART_OutUDec(channel);
          UART_OutString(") expected at least ");
          UART_OutUDec((unsigned long)previous);
          UART_OutString(" but measured ");
          UART_OutUDec((unsigned long)previous);
          UART_OutString(".\r\n");
          printLines = printLines + 1;
        }
      }
      previous = in;
      do{
                               // go to the next digital output pin
                               // if this pin is broken, go to the next one
                               // if this pin is the analog input, go to the next one
        outmask = outmask<<1;  // if run out of pins, break out of loop
        if(outmask == 0x10){
          outmask = 0x40;      // skip PD4 and PD5
        }
      } while(((digitalD&outmask) || (outmask == inmask)) && (outmask <= 0x80));
    }
    inmask = inmask<<1;        // check next analog input
    channel = channel - 1;     // check next analog input
  }
  GPIO_PORTD &= ~0xCF;         // PD = 0
  GPIO_PORTD_DIR_R &= ~0xCF;   // make PD7-6 and PD3-0 in
  GPIO_PORTD_AFSEL_R &= ~0xCF; // disable alt funct on PD7-6 and PD3-0
  GPIO_PORTD_DEN_R |= 0xCF;    // enable digital I/O on PD7-6 and PD3-0
  GPIO_PORTD_AMSEL_R &= ~0x0F; // disable analog functionality on PD3-0 (AIN4, AIN5, AIN6, and AIN7)
  // 19) test Port E analog (PE5=AIN8, PE4=AIN9, PE3=AIN0, PE2=AIN1, PE1=AIN2, and PE0=AIN3)
  inmask = 0x01;               // bit mask of analog input being tested
  channel = 3;                 // channel number of analog input being tested
  while(inmask <= 0x20){
                               // make PE5-0 out
    GPIO_PORTE_DIR_R |= (~inmask)&0x3F;
                               // disable alt funct on PE5-0
    GPIO_PORTE_AFSEL_R &= ~0x3F;
                               // enable digital I/O on PE5-0
    GPIO_PORTE_DEN_R |= (~inmask)&0x3F;
                               // disable analog functionality on PE5-0 (AIN8, AIN9, AIN0, AIN1, AIN2, and AIN3)
    GPIO_PORTE_AMSEL_R &= ~0x3F;
    GPIO_PORTE = 0;            // PE = 0
                               // make AIN[channel] analog in
    ADC0_InitSWTriggerSeq3(channel);
                               // short (possibly unnecessary) delay
    for(delay=0; delay<MAXDELAY; delay=delay+1){};
    previous = ADC0_InSeq3();  // take ADC sample
                               // ***Important: This should measure approximately zero.
                               // If a digital output pin is broken high (always +3.3),
                               // this will be non-zero and may cause a false-positive
                               // error.
    outmask = 0x01;            // initial output to set
                               // if this output is broken, go to the next one
                               // if they are all broken, this test will do nothing
    while(((digitalE&outmask) || (outmask == inmask)) && (outmask <= 0x20)){
      outmask = outmask<<1;
    }
    while(outmask <= 0x20){
      GPIO_PORTE |= outmask;   // set the next output pin high
                               // short (possibly unnecessary) delay
      for(delay=0; delay<MAXDELAY; delay=delay+1){};
      in = ADC0_InSeq3();      // take ADC sample
      if(in <= previous){      // basic linearity test; samples should increase
        analogE |= inmask;     // PE[inmask] analog input broken
        if(printLines < MAXLINES){
          UART_OutString("ADC input PE");
          switch(channel){
            case 0: UART_OutChar('3'); break;
            case 1: UART_OutChar('2'); break;
            case 2: UART_OutChar('1'); break;
            case 3: UART_OutChar('0'); break;
            case 8: UART_OutChar('5'); break;
            case 9: UART_OutChar('4'); break;
            default: UART_OutChar('?');
          }
          UART_OutString(" (AIN");
          UART_OutUDec(channel);
          UART_OutString(") expected at least ");
          UART_OutUDec((unsigned long)previous);
          UART_OutString(" but measured ");
          UART_OutUDec((unsigned long)previous);
          UART_OutString(".\r\n");
          printLines = printLines + 1;
        }
      }
      previous = in;
      do{
                               // go to the next digital output pin
                               // if this pin is broken, go to the next one
                               // if this pin is the analog input, go to the next one
        outmask = outmask<<1;  // if run out of pins, break out of loop
      } while(((digitalE&outmask) || (outmask == inmask)) && (outmask <= 0x20));
    }
    inmask = inmask<<1;        // check next analog input
    if(channel > 0){           // check next analog input
      channel = channel - 1;
    } else{
      channel = 9;
    }
  }
  GPIO_PORTE = 0;              // PE = 0
  GPIO_PORTE_DIR_R &= ~0x3F;   // make PE5-0 in
  GPIO_PORTE_AFSEL_R &= ~0x3F; // disable alt funct on PE5-0
  GPIO_PORTE_DEN_R |= 0x3F;    // enable digital I/O on PE5-0
  GPIO_PORTE_AMSEL_R &= ~0x3F; // disable analog functionality on PE5-0 (AIN8, AIN9, AIN0, AIN1, AIN2, and AIN3)
  // 20) output the results
  UART_OutString("\r\nSummary:\r\n");
  UART_OutString("\r\nAnalog:");
  UART_OutString("\r\nPort B: ");
  if(analogB == 0){
    UART_OutString("no errors");
  } else{
    if(analogB&0x10){
      UART_OutString("B4 (AIN10) ");
    }
    if(analogB&0x20){
      UART_OutString("B5 (AIN11) ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort D: ");
  if(analogD == 0){
    UART_OutString("no errors");
  } else{
    if(analogD&0x01){
      UART_OutString("D0 (AIN7) ");
    }
    if(analogD&0x02){
      UART_OutString("D1 (AIN6) ");
    }
    if(analogD&0x04){
      UART_OutString("D2 (AIN5) ");
    }
    if(analogD&0x08){
      UART_OutString("D3 (AIN4) ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort E: ");
  if(analogE == 0){
    UART_OutString("no errors");
  } else{
    if(analogE&0x01){
      UART_OutString("E0 (AIN3) ");
    }
    if(analogE&0x02){
      UART_OutString("E1 (AIN2) ");
    }
    if(analogE&0x04){
      UART_OutString("E2 (AIN1) ");
    }
    if(analogE&0x08){
      UART_OutString("E3 (AIN0) ");
    }
    if(analogE&0x10){
      UART_OutString("E4 (AIN9) ");
    }
    if(analogE&0x20){
      UART_OutString("E5 (AIN8) ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nDigital:");
  UART_OutString("\r\nPort A: ");
  if(digitalA == 0){
    UART_OutString("no errors");
  } else{
    if(digitalA&0x04){
      UART_OutString("A2 ");
    }
    if(digitalA&0x08){
      UART_OutString("A3 ");
    }
    if(digitalA&0x10){
      UART_OutString("A4 ");
    }
    if(digitalA&0x20){
      UART_OutString("A5 ");
    }
    if(digitalA&0x40){
      UART_OutString("A6 ");
    }
    if(digitalA&0x80){
      UART_OutString("A7 ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort B: ");
  if(digitalB == 0){
    UART_OutString("no errors");
  } else{
    if(digitalB&0x01){
      UART_OutString("B0 ");
    }
    if(digitalB&0x02){
      UART_OutString("B1 ");
    }
    if(digitalB&0x04){
      UART_OutString("B2 ");
    }
    if(digitalB&0x08){
      UART_OutString("B3 ");
    }
    if(digitalB&0x10){
      UART_OutString("B4 ");
    }
    if(digitalB&0x20){
      UART_OutString("B5 ");
    }
    if(digitalB&0x40){
      UART_OutString("B6 ");
    }
    if(digitalB&0x80){
      UART_OutString("B7 ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort C: ");
  if(digitalC == 0){
    UART_OutString("no errors");
  } else{
    if(digitalC&0x10){
      UART_OutString("C4 ");
    }
    if(digitalC&0x20){
      UART_OutString("C5 ");
    }
    if(digitalC&0x40){
      UART_OutString("C6 ");
    }
    if(digitalC&0x80){
      UART_OutString("C7 ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort D: ");
  if(digitalD == 0){
    UART_OutString("no errors");
  } else{
    if(digitalD&0x01){
      UART_OutString("D0 ");
    }
    if(digitalD&0x02){
      UART_OutString("D1 ");
    }
    if(digitalD&0x04){
      UART_OutString("D2 ");
    }
    if(digitalD&0x08){
      UART_OutString("D3 ");
    }
    if(digitalD&0x40){
      UART_OutString("D6 ");
    }
    if(digitalD&0x80){
      UART_OutString("D7 ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort E: ");
  if(digitalE == 0){
    UART_OutString("no errors");
  } else{
    if(digitalE&0x01){
      UART_OutString("E0 ");
    }
    if(digitalE&0x02){
      UART_OutString("E1 ");
    }
    if(digitalE&0x04){
      UART_OutString("E2 ");
    }
    if(digitalE&0x08){
      UART_OutString("E3 ");
    }
    if(digitalE&0x10){
      UART_OutString("E4 ");
    }
    if(digitalE&0x20){
      UART_OutString("E5 ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\nPort F: ");
  if(digitalF == 0){
    UART_OutString("no errors");
  } else{
    if(digitalF&0x01){
      UART_OutString("F0 ");
    }
    if(digitalF&0x02){
      UART_OutString("F1 ");
    }
    if(digitalF&0x04){
      UART_OutString("F2 ");
    }
    if(digitalF&0x08){
      UART_OutString("F3 ");
    }
    if(digitalF&0x10){
      UART_OutString("F4 ");
    }
    UART_OutString("broken");
  }
  UART_OutString("\r\n");
  if((digitalA == 0xFC) && (digitalB == 0xFF) && (digitalC == 0xF0) && (digitalD == 0xCF) && (digitalE == 0x3F) && (digitalF == 0x1F)){
    // Every pin appears to be broken, but the processor is definitely
    // running code.  The user probably did not realize that this
    // program requires a special custom Booster Pack to run (see
    // details in attached schematic).  Or the Booster Pack is plugged
    // in backwards.  Try to print a helpful message.
    UART_OutString("This program needs the LM4F120 tester Booster\r\n");
    UART_OutString("Pack to be correctly connected to your LaunchPad\r\n");
    UART_OutString("board.  If the LED is on, then it is plugged in\r\n");
    UART_OutString("backwards.  Unplug the USB cable from your\r\n");
    UART_OutString("computer and plug the Booster Pack the other way.\r\n");
    UART_OutString("The text on the Booster Pack should be going the\r\n");
    UART_OutString("same way as the text on the LaunchPad.\r\n");
  }
//  if(NYI){
    // This particular pattern of broken pins may be the result of the
    // Booster Pack being plugged in "one-off" from the correct
    // orientation.  Try to print a helpful message.
//  }
  // 21) flash the LED
  GPIO_PORTF_DIR_R |= 0x0E;        // make PF3-1 output (PF3-1 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0x0E;     // disable alt funct on PF3-1
  GPIO_PORTF_DEN_R |= 0x0E;        // enable digital I/O on PF3-1
                                   // configure PF3-1 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFF000F)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;          // disable analog functionality on PF
  LEDS = 0;                        // turn all LEDs off
  if((digitalA == 0xFC) && (digitalB == 0xFF) && (digitalC == 0xF0) && (digitalD == 0xCF) && (digitalE == 0x3F) && (digitalF == 0x1F)){
    // Booster Pack is backwards or missing -- flash red
    Timer0_Init(&FlashRed, 50000000);// initialize timer0 (1 Hz)
  } else if((digitalA > 0) || (digitalB > 0) || (digitalC > 0) || (digitalD > 0) || (digitalE > 0) || (digitalF > 0)){
    // A few errors and a few working pins -- rapidly flash yellow
    Timer0_Init(&FlashYellow, 5000000);// initialize timer0 (10 Hz)
  } else if((digitalA == 0) && (digitalB == 0) && (digitalC == 0) && (digitalD == 0) && (digitalE == 0) && (digitalF == 0)){
    // No errors -- flash green
    Timer0_Init(&FlashGreen, 50000000);// initialize timer0 (1 Hz)
  }
  while(1){
    WaitForInterrupt();
  }
}
