// WS2812TestMain.c
// Runs on TM4C123
// Test the functions in WS2812.c by sending various patterns to
// the string of LEDs, observe the results, and measure timing.
// Daniel Valvano
// June 24, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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

// WS2812 data connected to PE0 through 300 to 500 ohm resistor and level shifter circuit
// 1,000 uF, 6.3 V capacitor across power rails

// ****************************************
// **** Old Instructions for Reference ****
// ****************************************
// **** (old) Hook-Up and Power-Up Procedure (old) ****
// 1) Before applying power, connect the following together:
//    *WS2812 LED string +5 V, LaunchPad VBUS (+5 V), level shifter HV
//    *WS2812 LED string GND, LaunchPad GND, level shifter GND, black banana plug to power supply -, middle pin of 78S05 regulator
//    *LaunchPad +3.3 V, level shifter LV
//    *LaunchPad PE0, level shifter LV1
//    *level shifter LV1 to one side of 330 ohm resistor
//    *other side of 330 ohm resistor to WS2812 LED string DIN
//    *>=1,000 uF capacitor between right and middle pin of 78S05 regulator (check polarity)
//    *4.7 uF capacitor between left and middle pin of 78S05 regulator (check polarity)
// 2a) Before applying power, connect the following together to use the regulator:
//    *right pin of 78S05 regulator to +5 V from step 1)
//    *left pin of 78S05 regulator to power supply + red banana plug
//    *disconnect red banana plug from power supply + for now
// 2b) Before applying power, connect the following together to bypass the regulator:
//    *power supply + red banana plug to +5 V from step 1)
//    *disconnect red banana plug from power supply + for now
// 3a) Turn on power supply, turn on output, set voltage to 7.4 V, connect red banana plug to power supply +
// 3b) Turn on power supply, turn on output, set voltage to 5.0 V, connect red banana plug to power supply +

// **** (old) Unhook and Power-Down Procedure (old) ****
// 1) Turn off power supply
// 2) Disconnect red banana plug from power supply +

// **** (old) Programming Procedure (old) ****
// 1) Turn off power supply output
// 2) Disconnect LaunchPad VBUS (+5 V), LaunchPad +3.3 V, LaunchPad PE0
// 3) Connect LaunchPad to PC and program
// 4) Disconnect LaunchPad from PC
// 5) Repeat Power-Up procedure
// ***********************************************
// **** End of Old Instructions for Reference ****
// ***********************************************

// ******************************
// **** Instructions for Use ****
// ******************************
// **** Normal Power-Up Procedure (Powered from Battery) ****
// 1) Insert jumper near battery.
// 2) Connect battery to white, polarized connector.
// 3) Turn on power switch.
// 4) Press SW1 or SW2 on the LaunchPad.

// **** Normal Power-Down Procedure ****
// 1) Turn off power switch.

// **** Programming Procedure ****
// 0) Do not program the microcontroller to initially turn on more than about
//    500 mA of LEDs (about 25 colors at full brightness or 8 LEDs at full
//    white).  While being programmed, the LaunchPad is powered by USB by
//    default, and the USB powers the rest of the system.  Drawing more
//    current may overload the USB.
//    If this causes a problem, disconnect the LaunchPad from the Booster
//    Pack connector or disconnect the strings of LEDs before connecting the
//    LaunchPad to the PC.  Newer LaunchPads may have the option to be
//    powered through USB or the Booster Pack, but this feature was not
//    tested.
// 1) Disconnect battery from white, polarized connector.
// 2) Remove jumper near battery.
// 3) Connect LaunchPad to PC and program.  Do not program the
//    microcontroller to continuously turn on more than about 1 A of LEDs
//    (about 50 colors at full brightness or about 16 LEDs at full white).
// 4) Disconnect LaunchPad from PC.
// 5) Insert jumper near battery.
// 6) Connect battery to white, polarized connector.
// Alternatively,
// 1) Turn off power switch.
// 2) Disconnect LaunchPad from the Booster Pack.
// 3) Connect LaunchPad to PC and program.
// 4) Disconnect LaunchPad from the PC.
// 5) Connect LaunchPad to Booster Pack.
// *************************************
// **** End of Instructions for Use ****
// *************************************

#include <stdint.h>
#include "ADCSWTrigger.h"
#include "ADCT0ATrigger.h"
#include "WS2812.h"

#define GPIO_PORTF_DATA_R       (*((volatile uint32_t *)0x400253FC))
#define LEDS                    (*((volatile uint32_t *)0x40025038))
#define RED       0x02
#define BLUE      0x04
#define GREEN     0x08
#define SWITCHES                (*((volatile uint32_t *)0x40025044))
#define SW1       0x10                      // on the left side of the LaunchPad board
#define SW2       0x01                      // on the right side of the LaunchPad board
#define GPIO_PORTF_DIR_R        (*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile uint32_t *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile uint32_t *)0x40025520))
#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define GPIO_PORTF_CR_R         (*((volatile uint32_t *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile uint32_t *)0x4002552C))
#define TIMER1_CFG_R            (*((volatile uint32_t *)0x40031000))
#define TIMER1_TAMR_R           (*((volatile uint32_t *)0x40031004))
#define TIMER1_CTL_R            (*((volatile uint32_t *)0x4003100C))
#define TIMER1_IMR_R            (*((volatile uint32_t *)0x40031018))
#define TIMER1_ICR_R            (*((volatile uint32_t *)0x40031024))
#define TIMER_ICR_TATOCINT      0x00000001  // GPTM Timer A Time-Out Raw
                                            // Interrupt
#define TIMER1_TAILR_R          (*((volatile uint32_t *)0x40031028))
#define TIMER1_TAPR_R           (*((volatile uint32_t *)0x40031038))
#define SYSCTL_RCGCTIMER_R      (*((volatile uint32_t *)0x400FE604))
#define SYSCTL_RCGCTIMER_R1     0x00000002  // Timer 1 Run Mode Clock Gating
                                            // Control
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_RCGCGPIO_R5      0x00000020  // GPIO Port F Run Mode Clock
                                            // Gating Control
#define SYSCTL_PRTIMER_R        (*((volatile uint32_t *)0x400FEA04))
#define SYSCTL_PRTIMER_R1       0x00000002  // Timer 1 Peripheral Ready
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRGPIO_R5        0x00000020  // GPIO Port F Peripheral Ready
#define NVIC_ST_CURRENT_R       (*((volatile uint32_t *)0xE000E018))
#define NVIC_EN0_R              (*((volatile uint32_t *)0xE000E100))
#define NVIC_PRI5_R             (*((volatile uint32_t *)0xE000E414))

// functions defined in startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
int32_t StartCritical(void);  // previous I bit, disable interrupts
void EndCritical(int32_t sr); // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

// delay function from sysctl.c
// which delays 3*ulCount cycles
void Delay(uint32_t ulCount);

//------------Pattern_Reset------------
// Reset global variables associated with all test pattern
// functions.  This should be called when transitioning from one
// pattern to the next to prevent brief errors.
// Input: none
// Output: none
void Pattern_Reset(void);

//------------various test patterns------------
// Draw various test patterns on the string of LEDs.  Each call
// draws the pattern once by updating the RAM buffer and sending
// it once.  An additional external delay may be necessary if
// calling these functions repeatedly.
// Input: scale  LED outputs are right shifted by this amount (0<=scale<=7) to reduce current/brightness
//        row    index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_RainbowWaves(uint8_t scale, uint8_t cols, uint8_t row);
void Pattern_WhiteWaves(uint8_t scale, uint8_t row);
void Pattern_RedWhiteBlue(uint8_t scale, uint8_t row);
void Pattern_Stacker(uint8_t red, uint8_t green, uint8_t blue, uint32_t delay, uint8_t row);
void Pattern_ThreeRowSquare(uint8_t waveRed, uint8_t waveGreen, uint8_t waveBlue, uint8_t bgRed, uint8_t bgGreen, uint8_t bgBlue);
void Pattern_ThreeRowTriangle(uint8_t waveRed, uint8_t waveGreen, uint8_t waveBlue, uint8_t bgRed, uint8_t bgGreen, uint8_t bgBlue);
void Pattern_ThreeRowSine(uint8_t waveRed, uint8_t waveGreen, uint8_t waveBlue, uint8_t bgRed, uint8_t bgGreen, uint8_t bgBlue);
void Pattern_PwrMeterBasic(uint8_t litLEDs, uint8_t row);
void Pattern_PwrMeterStriped(uint8_t litLEDs, uint8_t row);
void Pattern_PwrMeterColored(uint8_t litLEDs, uint8_t row);
void Pattern_SplitMeterBasic(uint8_t litLEDs, uint8_t LEDsPerSide, uint8_t row);
void Pattern_SplitMeterStriped(uint8_t litLEDs, uint8_t LEDsPerSide, uint8_t row);
void Pattern_SplitMeterColored(uint8_t litLEDs, uint8_t LEDsPerSide, uint8_t row);

//------------global variables for patterns------------
// These could be made private when done debugging.
uint8_t phase = 0;          // phase value of the LED pattern
uint8_t red, green, blue;   // value of each color
int32_t ired, igreen, iblue;// amount to increment each color

//------------global variables for random number generator------------
// These could be made private when done debugging.
// Linear congruential generator from Numerical Recipes
// by Press et al.  To use this module, call Random_Init()
// once with a seed and (Random()>>24)%60 over and over to
// get a new random number from 0 to 59.
uint32_t M;

//------------Random_Init------------
// Initialize the random number generator with the given seed.
// Input: seed  new seed value for random number generation
// Output: none
void Random_Init(uint32_t seed){
  M = seed;
}

//------------Random------------
// Generate a semi-random number.  The lower bits might not be
// random, so right shift by some amount before scaling to the
// desired range.
// Input: none
// Output: 32-bit semi-random number
uint32_t Random(void){
  M = 1664525*M+1013904223;
  return(M);
}

uint32_t LastX, LastY, LastZ;// previous acceleration measurement
uint32_t AccX, AccY, AccZ;  // most recent acceleration measurement
void MeasureAcceleration(uint32_t x, uint32_t y, uint32_t z){
  LEDS ^= BLUE;             // toggle blue LED (PF2)
  LastX = AccX;
  LastY = AccY;
  LastZ = AccZ;
  AccX = x;
  AccY = y;
  AccZ = z;
}

uint32_t Sound[16];         // buffer of audio samples
uint32_t SoundIndex = 0;    // index into buffer of audio sample
void MeasureMicrophone(uint32_t microphone){
  LEDS ^= GREEN;            // toggle green LED (PF3)
  if(SoundIndex >= 16){
    SoundIndex = 0;
  }
  Sound[SoundIndex] = microphone;
  SoundIndex = SoundIndex + 1;
}

void Timer1A_Handler(void){
  TIMER1_ICR_R = TIMER_ICR_TATOCINT;// acknowledge timer0A timeout
  ADC0_InSeq2();            // take accelerometer measurement
}

int main(void){
  uint32_t i, run, maxrun;
  // **** LED string initialization ****
  WS2812_Init();            // set system clock to 80 MHz, initialize SysTick, initialize PE0 and make it output
  // **** ADC0 initialization ****
                            // sample accelerometer on ADC channels 6, 5, and 4 when triggered by software
  ADC0_InitSWTriggerSeq2(&MeasureAcceleration);
                            // sample microphone on ADC channel 7 when triggered by 12,800 Hz Timer0
  ADC0_InitTimer0ATriggerSeq3(6250, &MeasureMicrophone);
  // **** GPIO Port F initialization ****
                            // activate clock for Port F
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
                            // allow time for clock to stabilize
  while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R5) == 0){};
                            // unlock GPIO Port F Commit Register
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
  GPIO_PORTF_CR_R |= 0x1F;  // enable commit for PF4-0
  GPIO_PORTF_DIR_R &= ~0x11;// make PF4,0 in (PF4,0 built-in buttons)
  GPIO_PORTF_DIR_R |= 0x0E; // make PF3-1 out (PF3-0 built-in LEDs)
  GPIO_PORTF_AFSEL_R &= ~0x1F;// disable alt funct on PF4-0
  GPIO_PORTF_PUR_R |= 0x11; // enable pull-up on PF4,0
  GPIO_PORTF_DEN_R |= 0x1F; // enable digital I/O on PF4-0
                            // configure PF4-0 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFF00000)+0x00000000;
  LEDS = 0;
  // **** Timer1A initialization ****
                            // activate clock for Timer1
  SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
                            // allow time for clock to stabilize
  while((SYSCTL_PRTIMER_R&SYSCTL_PRTIMER_R1) == 0){};
  TIMER1_CTL_R &= ~0x0001;  // disable Timer1A during setup
  TIMER1_CFG_R = 0x0004;    // configure for 16-bit timer mode
  TIMER1_TAMR_R = 0x0002;   // configure for periodic mode, default down-count settings
  TIMER1_TAPR_R = 24;       // prescale value for 50 Hz interrupt
  TIMER1_TAILR_R = 63999;   // reload value for 50 Hz interrupt
  TIMER1_ICR_R = 0x0001;    // clear TIMER1A timeout flag
  TIMER1_IMR_R |= 0x0001;   // arm timeout interrupt
  // **** NVIC interrupt initialization ****
  NVIC_PRI5_R = (NVIC_PRI5_R&0xFFFF00FF)|0x00004000; // priority 2
  NVIC_EN0_R = 1<<21;       // enable interrupt 21 in NVIC
                            // Timer5A interrupt on DMA completion;
                            // ADC0 Sequencer 3 interrupt on microphone sample complete;
  EnableInterrupts();       // Timer1A interrupt at 50 Hz accelerometer sampling frequency
  WS2812_ClearBuffer();
  WS2812_SendReset();
  WS2812_AddColor(0xAA, 0xAA, 0xAA, 0);
  WS2812_AddColor(64, 0, 0, 0);
  WS2812_AddColor(128, 0, 0, 0);
  WS2812_AddColor(64, 64, 0, 0);
  WS2812_AddColor(0, 64, 0, 0);
  WS2812_AddColor(0, 128, 0, 0);
  WS2812_AddColor(0, 64, 64, 0);
  WS2812_AddColor(0, 0, 64, 0);
  WS2812_AddColor(0, 0, 128, 0);
  WS2812_AddColor(64, 0, 64, 0);
  WS2812_AddColor(32, 32, 32, 0);
  WS2812_AddColor(64, 64, 64, 0);
  WS2812_AddColor(96, 96, 96, 0);
  WS2812_AddColor(128, 128, 128, 0);
  WS2812_AddColor(0, 0, 0, 1);
  WS2812_AddColor(0, 128, 0, 1);   // second LED of row 1 green
  WS2812_AddColor(0, 0, 0, 2);
  WS2812_AddColor(0, 0, 0, 2);
  WS2812_AddColor(128, 0, 0, 2);   // third LED of row 2 red
  WS2812_PrintBuffer();
  while(SWITCHES == (SW1|SW2)){};  // wait for switch pressed
  Delay(6666666);                  // debounce 0.25 sec
  while(SWITCHES != (SW1|SW2)){};  // wait for switch released
  while(1){
    
//    run = 0;
//    maxrun = 80;
//    Pattern_Reset();
//    WS2812_ClearBuffer();
//    while((SWITCHES == (SW1|SW2) && (run < maxrun))){ // repeat while switches released or for about 20 seconds
//      Pattern_RainbowWaves(1, NUMCOLS, 0);
//      Delay(6666666);              // delay about 0.25 sec
//      run = run + 1;
//    }
//    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
/*temp long test*/
//Test the dynamic Pattern_RainbowWaves() function with rainbows of
//increasing length.  Every time the button is pressed, the length
//of the rainbow on row 0 increases by 1, and the number of red
//LEDs on row 1 increases by 1.  The button may need to be pressed
//three times before a rainbow actually appears on row 0, since a
//rainbow cannot be formed with fewer than three colors.  A color
//is not necessarily added to the rainbow each time the length of
//the rainbow is incremented.
/*    i = 0;
    WS2812_SetCursor(0, 1);
    while(i <= 24){
      Pattern_Reset();
      while(SWITCHES == (SW1|SW2)){// repeat while switches released
        Pattern_RainbowWaves(1, i, 0);
        Delay(6666666);            // delay about 0.25 sec
      }
      while(SWITCHES != (SW1|SW2)){};// wait for switch released
      i = i + 1;
      WS2812_AddColor(64, 0, 0, 1);// light another LED on row 1
      WS2812_SendReset();
      WS2812_PrintBuffer();
    }*/
/*end of temp long test*/

//    run = 0;
//    maxrun = 80;
//    Pattern_Reset();
//    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 20 seconds
//      Pattern_WhiteWaves(2, 0);
//      Delay(6666666);              // delay about 0.25 sec
//      run = run + 1;
//    }
//    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
    
//    run = 0;
//    maxrun = 200;
//    Pattern_Reset();
//    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 20 seconds
//      Pattern_RedWhiteBlue(1, 0);
//      Delay(2666666);              // delay about 0.1 sec
//      run = run + 1;
//    }
//    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
    
//    run = 0;
//    maxrun = 3;
//    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or until sequence completes 3 times
//      Pattern_Reset();
//      WS2812_ClearBuffer();
//      i = 0;
//      while((i < NUMCOLS) && (SWITCHES == (SW1|SW2))){
//        i = i + 1;
//        Pattern_Stacker(255, 0, 0, 80000, 0);
//      }
//      Pattern_Reset();
//      WS2812_ClearBuffer();
//      Random_Init(NVIC_ST_CURRENT_R);
//      i = 0;
//      while((i < NUMCOLS) && (SWITCHES == (SW1|SW2))){
//        i = i + 1;
//        Pattern_Stacker(Random()>>24, Random()>>24, Random()>>24, 266666, 0);
//      }
//      if(i >= NUMCOLS){            // skip the delay if the above loops broke due to button pressed
//        Delay(133333333);          // delay about 5 sec to observe random strand
//      }
//      run = run + 1;
//    }
//    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
// squarewave    
    run = 0;
    maxrun = 80;
    Pattern_Reset();
    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 20 seconds
      Pattern_ThreeRowSquare(0, 128, 255, 12, 0, 0);
      Delay(6666666);              // delay about 0.25 sec
      run = run + 1;
    }
    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    run = 0;
    maxrun = 80;
    Pattern_Reset();
    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 20 seconds
      Pattern_ThreeRowTriangle(255, 255, 0, 0, 0, 12);
      Delay(6666666);              // delay about 0.25 sec
      run = run + 1;
    }
    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
// sine wave    
    i = 0;
    run = 0;
    maxrun = 80;
    Pattern_Reset();
    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 20 seconds
      if(i < 25){
        red = 250 - 10*i;
        green = 10*i;
        blue = 0;
        ired = -10;
        igreen = 10;
        iblue = 0;
      } else if(i < 50){
        red = 0;
        green = 250 - 10*(i - 25);
        blue = 10*(i - 25);
        ired = 0;
        igreen = -10;
        iblue = 10;
      } else{
        red = 10*(i - 50);
        green = 0;
        blue = 250 - 10*(i - 50);
        ired = 10;
        igreen = 0;
        iblue = -10;
      }
      i = i + 1;
      if(i > 74){
        i = 0;
      }
      Pattern_ThreeRowSine(red, green, blue, 0, 0, 0);
      red = red + ired;
      green = green + igreen;
      blue = blue + iblue;
      if((red == 250) && (ired > 0)){
        igreen = ired;
        ired = -1*ired;
      }
      if((red == 0) && (ired < 0)){
        ired = 0;
      }
      if((green == 250) && (igreen > 0)){
        iblue = igreen;
        igreen = -1*igreen;
      }
      if((green == 0) && (igreen < 0)){
        igreen = 0;
      }
      if((blue == 250) && (iblue > 0)){
        ired = iblue;
        iblue = -1*iblue;
      }
      if((blue == 0) && (iblue < 0)){
        iblue = 0;
      }
      Delay(6666666);              // delay about 0.25 sec
      run = run + 1;
    }
    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
    
    run = 0;
    maxrun = 2;
    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 18 seconds
      i = 0;
      while((i < 36) && (SWITCHES == (SW1|SW2))){
        Pattern_PwrMeterBasic(i, 0);
        Pattern_PwrMeterStriped(i, 1);
        Pattern_PwrMeterColored(i, 2);
        Delay(3333333);            // delay about 0.125 sec
        i = i + 1;
      }
      i = 36;
      while((i > 0) && (SWITCHES == (SW1|SW2))){
        Pattern_PwrMeterBasic(i, 0);
        Pattern_PwrMeterStriped(i, 1);
        Pattern_PwrMeterColored(i, 2);
        Delay(3333333);            // delay about 0.125 sec
        i = i - 1;
      }
      run = run + 1;
    }
    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    run = 0;
    maxrun = 4;
    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 18 seconds
      i = 0;
      while((i < 18) && (SWITCHES == (SW1|SW2))){
        Pattern_SplitMeterBasic(i, 18, 0);
        Pattern_SplitMeterStriped(i, 18, 1);
        Pattern_SplitMeterColored(i, 18, 2);
        Delay(3333333);            // delay about 0.125 sec
        i = i + 1;
      }
      i = 18;
      while((i > 0) && (SWITCHES == (SW1|SW2))){
        Pattern_SplitMeterBasic(i, 18, 0);
        Pattern_SplitMeterStriped(i, 18, 1);
        Pattern_SplitMeterColored(i, 18, 2);
        Delay(3333333);            // delay about 0.125 sec
        i = i - 1;
      }
      run = run + 1;
    }
    while(SWITCHES != (SW1|SW2)){};// wait for switch released
    
    
    run = 0;
    maxrun = 80;
    TIMER1_CTL_R |= 0x0001;        // enable Timer1A 16b, periodic, interrupts
    while((SWITCHES == (SW1|SW2)) && (run < maxrun)){ // repeat while switches released or for about 20 seconds
      if(AccX > LastX){            // simple absolute value implementation
        Pattern_SplitMeterColored(((AccX - LastX)>>1), 18, 0);
      } else{
        Pattern_SplitMeterColored(((LastX - AccX)>>1), 18, 0);
      }
      if(AccY > LastY){            // simple absolute value implementation
        Pattern_SplitMeterColored(((AccY - LastY)>>1), 18, 1);
      } else{
        Pattern_SplitMeterColored(((LastY - AccY)>>1), 18, 1);
      }
      if(AccZ > LastZ){            // simple absolute value implementation
        Pattern_SplitMeterColored(((AccZ - LastZ)>>1), 18, 2);
      } else{
        Pattern_SplitMeterColored(((LastZ - AccZ)>>1), 18, 2);
      }
      Delay(6666666);              // delay about 0.25 sec
      run = run + 1;
    }
    TIMER1_CTL_R &= ~0x0001;       // disable Timer1A
    while(SWITCHES != (SW1|SW2)){};// wait for switch released
  }
}

//------------Pattern_Reset------------
// Reset global variables associated with all test pattern
// functions.  This should be called when transitioning from one
// pattern to the next to prevent brief errors.
// Input: none
// Output: none
void Pattern_Reset(void){
  phase = 0;
}

//------------Pattern_RainbowWaves------------
// Draw a rainbow pattern on the string of LEDs.  Each call
// draws the pattern once by updating the RAM buffer and sending
// it once.  An additional external delay is advised.
// Input: scale  LED outputs are right shifted by this amount (0<=scale<=7) to reduce current/brightness
//        cols   number of LEDs per rainbow (3<=cols)
//        row    index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_RainbowWaves(uint8_t scale, uint8_t cols, uint8_t row){
  uint32_t i;
  int32_t increment, maximum;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  if(cols < 3){
    return;                        // LEDs per rainbow parameter is invalid
  }
  WS2812_SetCursor(0, row);
  cols = 3*(cols/3);               // round down to nearest multiple of 3
  increment = 255/(cols/3);        // constant amount of color added or subtracted from one LED to the next
  maximum = increment*cols/3;      // largest amount of color any LED will reach (i.e. (maximum, 0, 0) is the "reddest" LED)
  if(phase < (cols/3)){
    red = maximum - increment*phase;
    green = increment*phase;
    blue = 0;
    ired = -1*increment;
    igreen = increment;
    iblue = 0;
  } else if(phase < (2*cols/3)){
    red = 0;
    green = maximum - increment*(phase - (cols/3));
    blue = increment*(phase - (cols/3));
    ired = 0;
    igreen = -1*increment;
    iblue = increment;
  } else{
    red = increment*(phase - (2*cols/3));
    green = 0;
    blue = maximum - increment*(phase - (2*cols/3));
    ired = increment;
    igreen = 0;
    iblue = -1*increment;
  }
  phase = phase + 1;
  if(phase >= cols){
    phase = 0;
  }
  for(i=0; i<NUMCOLS; i=i+1){
    WS2812_AddColor(red>>scale, green>>scale, blue>>scale, row);
    red = red + ired;
    green = green + igreen;
    blue = blue + iblue;
    if((red == maximum) && (ired > 0)){
      igreen = ired;
      ired = -1*ired;
    }
    if((red == 0) && (ired < 0)){
      ired = 0;
    }
    if((green == maximum) && (igreen > 0)){
      iblue = igreen;
      igreen = -1*igreen;
    }
    if((green == 0) && (igreen < 0)){
      igreen = 0;
    }
    if((blue == maximum) && (iblue > 0)){
      ired = iblue;
      iblue = -1*iblue;
    }
    if((blue == 0) && (iblue < 0)){
      iblue = 0;
    }
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_WhiteWaves------------
// Draw a white wave pattern on the string of LEDs.  Each call
// draws the pattern once by updating the RAM buffer and sending
// it once.  An additional external delay is advised.
// Input: scale  LED outputs are right shifted by this amount (0<=scale<=7) to reduce current/brightness
//        row    index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_WhiteWaves(uint8_t scale, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  WS2812_SetCursor(0, row);
  if((phase%25) < 13){
    red = green = blue = 21*(phase%25);
  } else if((phase%25) == 13){
    red = green = blue = 252;
  } else{
    red = green = blue = 252 - 21*((phase - 13)%25);
  }
  if((phase%25) < 12){
    ired = igreen = iblue = 21;
  } else if((phase%25) == 12){
    ired = igreen = iblue = 0;
  } else{
    ired = igreen = iblue = -21;
  }
  phase = phase + 1;
  if(phase > 24){
    phase = 0;
  }
  for(i=0; i<NUMCOLS; i=i+1){
    WS2812_AddColor(red>>scale, green>>scale, blue>>scale, row);
    red = red + ired;
    green = green + igreen;
    blue = blue + iblue;
    if((red == 252) && (ired > 0)){
      ired = 0;
    } else if((red == 252) && (ired == 0)){
      ired = -21;
    } else if(red == 0){
      ired = -1*ired;
    }
    if((green == 252) && (igreen > 0)){
      igreen = 0;
    } else if((green == 252) && (igreen == 0)){
      igreen = -21;
    } else if(green == 0){
      igreen = -1*igreen;
    }
    if((blue == 252) && (iblue > 0)){
      iblue = 0;
    } else if((blue == 252) && (iblue == 0)){
      iblue = -21;
    } else if(blue == 0){
      iblue = -1*iblue;
    }
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_RedWhiteBlue------------
// Draw a red, white, and blue wave pattern on the string of
// LEDs.  Each call draws the pattern once by updating the RAM
// buffer and sending it once.  An additional external delay is
// advised.
// Input: scale  LED outputs are right shifted by this amount (0<=scale<=7) to reduce current/brightness
//        row    index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_RedWhiteBlue(uint8_t scale, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  WS2812_SetCursor(0, row);
  if((phase%21) < 3){
    red = 63*(phase%21 + 1);
    green = 0;
    blue = 0;
  } else if((phase%21) < 7){
    red = 252 - 63*(phase%21 - 3);
    green = 0;
    blue = 0;
  } else if((phase%21) < 11){
    red = 63*(phase%21 - 6);
    green = 63*(phase%21 - 6);
    blue = 63*(phase%21 - 6);
  } else if((phase%21) < 14){
    red = 252 - 63*(phase%21 - 10);
    green = 252 - 63*(phase%21 - 10);
    blue = 252 - 63*(phase%21 - 10);
  } else if((phase%21) < 18){
    red = 0;
    green = 0;
    blue = 63*(phase%21 - 13);
  } else{
    red = 0;
    green = 0;
    blue = 252 - 63*(phase%21 - 17);
  }
  if((phase%21) < 3){
    ired = 63;
    igreen = 0;
    iblue = 0;
  } else if((phase%21) < 6){
    ired = -63;
    igreen = 0;
    iblue = 0;
  } else if((phase%21) == 6){
    ired = 0;
    igreen = 63;
    iblue = 63;
  } else if((phase%21) < 10){
    ired = 63;
    igreen = 63;
    iblue = 63;
  } else if((phase%21) < 13){
    ired = -63;
    igreen = -63;
    iblue = -63;
  } else if((phase%21) == 13){
    ired = -63;
    igreen = -63;
    iblue = 0;
  } else if((phase%21) < 17){
    ired = 0;
    igreen = 0;
    iblue = 63;
  } else if((phase%21) < 20){
    ired = 0;
    igreen = 0;
    iblue = -63;
  } else{
    ired = 63;
    igreen = 0;
    iblue = -63;
  }
  phase = phase + 1;
  if(phase > 20){
    phase = 0;
  }
  for(i=phase; i<(NUMCOLS+phase); i=i+1){
    WS2812_AddColor(red>>scale, green>>scale, blue>>scale, row);
    red = red + ired;
    green = green + igreen;
    blue = blue + iblue;
    if((i%21) < 3){
      ired = 63;
      igreen = 0;
      iblue = 0;
    } else if((i%21) < 6){
      ired = -63;
      igreen = 0;
      iblue = 0;
    } else if((i%21) == 6){
      ired = 0;
      igreen = 63;
      iblue = 63;
    } else if((i%21) < 10){
      ired = 63;
      igreen = 63;
      iblue = 63;
    } else if((i%21) < 13){
      ired = -63;
      igreen = -63;
      iblue = -63;
    } else if((i%21) == 13){
      ired = -63;
      igreen = -63;
      iblue = 0;
    } else if((i%21) < 17){
      ired = 0;
      igreen = 0;
      iblue = 63;
    } else if((i%21) < 20){
      ired = 0;
      igreen = 0;
      iblue = -63;
    } else{
      ired = 63;
      igreen = 0;
      iblue = -63;
    }
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_Stacker------------
// Stack LEDs of the given color from end to beginning of the
// LED strand.  The color starts at the end of the strand at
// full brightness and shifts to the beginning.  It stops at the
// first unilluminated LED nearest to the beginning, leaving the
// LED illuminated with the given color divided by 8.  The
// divide-by-eight limits the total current after this function
// has been called repeatedly and helps to emphasize the next
// moving LED.  The color delays at least 3*delay cycles before
// moving one step toward the beginning.  This means that
// subsequent calls to this function take less time as the LED
// strand fills up and later colors travel fewer steps.
// Input: red   8-bit red color value
//        green 8-bit green color value
//        blue  8-bit blue color value
//        delay wait 3 times this number of clock cycles between animations
//        row   index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_Stacker(uint8_t red, uint8_t green, uint8_t blue, uint32_t delay, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  for(i=(NUMCOLS-1); i>phase; i=i-1){
    WS2812_SetCursor(i, row);
    WS2812_AddColor(red, green, blue, row);
    WS2812_SendReset();
    WS2812_PrintBuffer();
    Delay(delay);
    WS2812_SetCursor(i, row);
    WS2812_AddColor(0, 0, 0, row);
    WS2812_SendReset();
    WS2812_PrintBuffer();
  }
  WS2812_SetCursor(phase, row);
  WS2812_AddColor(red>>3, green>>3, blue>>3, row);
  WS2812_SendReset();
  WS2812_PrintBuffer();
  Delay(delay);
  phase = phase + 1;
}

//------------Pattern_ThreeRowSquare------------
// Draw a square wave across the bottom three rows of LEDs.
// Each call draws the pattern once by updating the RAM buffer
// and sending it once.  An additional external delay is
// advised.  The pattern is eight columns long and repeating.
// [*] [*] [*] [*] [*] [ ] [ ] [ ]
// [*] [ ] [ ] [ ] [*] [ ] [ ] [ ]
// [*] [ ] [ ] [ ] [*] [*] [*] [*]
// Input: waveRed   8-bit red color value for the wave
//        waveGreen 8-bit green color value for the wave
//        waveBlue  8-bit blue color value for the wave
//        bgRed     8-bit red color value for the background
//        bgGreen   8-bit green color value for the background
//        bgBlue    8-bit blue color value for the background
// Output: none
void Pattern_ThreeRowSquare(uint8_t waveRed, uint8_t waveGreen, uint8_t waveBlue, uint8_t bgRed, uint8_t bgGreen, uint8_t bgBlue){
  uint32_t total, divisor, i;
  // This pattern will assign a color to every LED, and there
  // are 3 individual colored LEDs per LED unit, NUMCOLS (see
  // definition in WS2812.h) LEDs per row/strand, 3
  // rows/strands, and each colored LED uses a maximum of 20
  // mA.  All LEDs at full brightness will draw 3*3*20*NUMCOLS
  // mA, which can easily exceed your system's specifications.
  // Instead, automatically scale down the foreground and
  // background brightness to keep current below about 750 mA.
  // On average, each column has 1.5 LEDs on and 1.5 LEDs off.
  // The total amount of color in all LEDs is:
  total = (waveRed + waveGreen + waveBlue + bgRed + bgGreen + bgBlue)*NUMCOLS;
  // An LED with color of 255 draws about 20 mA.  (But the
  // previous line really should have been multiplied by 1.5.
  // See the equivalent lines in Pattern_ThreeRowTriangle()
  // and Pattern_ThreeRowSine() for clearer numbers.)
  // 1.5 LEDs with color of 255 draw about 30 mA.
  // The total amount of current in all LEDs is approximately:
  total = total*30/255; // units of mA
  // Calculate the value to divide all colors by to decrease
  // the total current below 750 mA.
  divisor = total/750 + 1;
  // Update the colors by dividing by the divisor.
  waveRed = waveRed/divisor;
  waveGreen = waveGreen/divisor;
  waveBlue = waveBlue/divisor;
  bgRed = bgRed/divisor;
  bgGreen = bgGreen/divisor;
  bgBlue = bgBlue/divisor;
  // Draw the pattern, starting at the first column.
  WS2812_SetCursor(0, 0);
  WS2812_SetCursor(0, 1);
  WS2812_SetCursor(0, 2);
  for(i=0; i<NUMCOLS; i=i+1){
    if(((i + phase + 4)%8) < 5){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 0);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 0);
    }
    if(((i + phase)%4) == 0){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 1);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 1);
    }
    if(((i + phase)%8) < 5){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 2);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 2);
    }
  }
  phase = phase + 1;
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_ThreeRowTriangle------------
// Draw a triangle wave across the bottom three rows of LEDs.
// Each call draws the pattern once by updating the RAM buffer
// and sending it once.  An additional external delay is
// advised.  The pattern is four columns long and repeating.
// [ ] [ ] [*] [ ]
// [ ] [*] [ ] [*]
// [*] [ ] [ ] [ ]
// Input: waveRed   8-bit red color value for the wave
//        waveGreen 8-bit green color value for the wave
//        waveBlue  8-bit blue color value for the wave
//        bgRed     8-bit red color value for the background
//        bgGreen   8-bit green color value for the background
//        bgBlue    8-bit blue color value for the background
// Output: none
void Pattern_ThreeRowTriangle(uint8_t waveRed, uint8_t waveGreen, uint8_t waveBlue, uint8_t bgRed, uint8_t bgGreen, uint8_t bgBlue){
  uint32_t total, divisor, i;
  // This pattern will assign a color to every LED, and there
  // are 3 individual colored LEDs per LED unit, NUMCOLS (see
  // definition in WS2812.h) LEDs per row/strand, 3
  // rows/strands, and each colored LED uses a maximum of 20
  // mA.  All LEDs at full brightness will draw 3*3*20*NUMCOLS
  // mA, which can easily exceed your system's specifications.
  // Instead, automatically scale down the foreground and
  // background brightness to keep current below about 750 mA.
  // Each column has one LED on and two LEDs off.
  // The total amount of color in all LEDs is:
  total = (waveRed + waveGreen + waveBlue + 2*bgRed + 2*bgGreen + 2*bgBlue)*NUMCOLS;
  // An LED with color of 255 draws about 20 mA.
  // The total amount of current in all LEDs is approximately:
  total = total*20/255; // units of mA
  // Calculate the value to divide all colors by to decrease
  // the total current below 750 mA.
  divisor = total/750 + 1;
  // Update the colors by dividing by the divisor.
  waveRed = waveRed/divisor;
  waveGreen = waveGreen/divisor;
  waveBlue = waveBlue/divisor;
  bgRed = bgRed/divisor;
  bgGreen = bgGreen/divisor;
  bgBlue = bgBlue/divisor;
  // Draw the pattern, starting at the first column.
  WS2812_SetCursor(0, 0);
  WS2812_SetCursor(0, 1);
  WS2812_SetCursor(0, 2);
  for(i=0; i<NUMCOLS; i=i+1){
    if(((i + phase)%4) == 0){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 0);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 0);
    }
    if(((i + phase)%2) == 1){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 1);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 1);
    }
    if(((i + phase)%4) == 2){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 2);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 2);
    }
  }
  phase = phase + 1;
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_ThreeRowSine------------
// Draw a sine wave across the bottom three rows of LEDs.
// Each call draws the pattern once by updating the RAM buffer
// and sending it once.  An additional external delay is
// advised.  The pattern is six columns long and repeating.
// [ ] [ ] [ ] [*] [*] [ ]
// [ ] [ ] [*] [ ] [ ] [*]
// [*] [*] [ ] [ ] [ ] [ ]
// Input: waveRed   8-bit red color value for the wave
//        waveGreen 8-bit green color value for the wave
//        waveBlue  8-bit blue color value for the wave
//        bgRed     8-bit red color value for the background
//        bgGreen   8-bit green color value for the background
//        bgBlue    8-bit blue color value for the background
// Output: none
void Pattern_ThreeRowSine(uint8_t waveRed, uint8_t waveGreen, uint8_t waveBlue, uint8_t bgRed, uint8_t bgGreen, uint8_t bgBlue){
  uint32_t total, divisor, i;
  // This pattern will assign a color to every LED, and there
  // are 3 individual colored LEDs per LED unit, NUMCOLS (see
  // definition in WS2812.h) LEDs per row/strand, 3
  // rows/strands, and each colored LED uses a maximum of 20
  // mA.  All LEDs at full brightness will draw 3*3*20*NUMCOLS
  // mA, which can easily exceed your system's specifications.
  // Instead, automatically scale down the foreground and
  // background brightness to keep current below about 750 mA.
  // Each column has one LED on and two LEDs off.
  // The total amount of color in all LEDs is:
  total = (waveRed + waveGreen + waveBlue + 2*bgRed + 2*bgGreen + 2*bgBlue)*NUMCOLS;
  // An LED with color of 255 draws about 20 mA.
  // The total amount of current in all LEDs is approximately:
  total = total*20/255; // units of mA
  // Calculate the value to divide all colors by to decrease
  // the total current below 750 mA.
  divisor = total/750 + 1;
  // Update the colors by dividing by the divisor.
  waveRed = waveRed/divisor;
  waveGreen = waveGreen/divisor;
  waveBlue = waveBlue/divisor;
  bgRed = bgRed/divisor;
  bgGreen = bgGreen/divisor;
  bgBlue = bgBlue/divisor;
  // Draw the pattern, starting at the first column.
  WS2812_SetCursor(0, 0);
  WS2812_SetCursor(0, 1);
  WS2812_SetCursor(0, 2);
  for(i=0; i<NUMCOLS; i=i+1){
    if(((i + phase)%6) < 2){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 0);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 0);
    }
    if(((i + phase)%3) == 2){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 1);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 1);
    }
    if(((i + phase + 3)%6) < 2){
      WS2812_AddColor(waveRed, waveGreen, waveBlue, 2);
    } else{
      WS2812_AddColor(bgRed, bgGreen, bgBlue, 2);
    }
  }
  phase = phase + 1;
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_PwrMeterBasic------------
// Implement a power meter by illuminating a given number of
// LEDs, starting at the beginning of the strand.  The whole
// meter will always be the same constant color.
// Input: litLEDs number of LEDs to illuminate (litLEDs<=NUMCOLS)
//        row     index of LED row (0<=row<NUMROWS)
// Output: none
#define PWR_BASIC_RED 0
#define PWR_BASIC_GRN 0
#define PWR_BASIC_BLU 255
void Pattern_PwrMeterBasic(uint8_t litLEDs, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  WS2812_SetCursor(0, row);
  for(i=1; i<=NUMCOLS; i=i+1){
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(i < (litLEDs - 1)){  // LEDs near beginning of strand slightly on
      WS2812_AddColor(PWR_BASIC_RED>>3, PWR_BASIC_GRN>>3, PWR_BASIC_BLU>>3, row);
    } else if(i == (litLEDs - 1)){ // LED near end of bar slightly brighter
      WS2812_AddColor(PWR_BASIC_RED>>1, PWR_BASIC_GRN>>1, PWR_BASIC_BLU>>1, row);
    } else if(i == litLEDs){       // LED at end of bar full brightness
      WS2812_AddColor(PWR_BASIC_RED, PWR_BASIC_GRN, PWR_BASIC_BLU, row);
    } else{                        // LEDs beyond end of bar fully off
      WS2812_AddColor(0, 0, 0, row);
    }
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_PwrMeterStriped------------
// Implement a power meter by illuminating a given number of
// LEDs, starting at the beginning of the strand.  The meter
// will be striped, starting at the beginning of the strand.
// A fully illuminated striped power meter will be made of green
// LEDs followed by yellow, followed by orange, followed by red.
// A partially illuminated striped power meter will be made of
// green LEDs followed by yellow, possibly followed by orange.
// Input: litLEDs number of LEDs to illuminate (litLEDs<=NUMCOLS)
//        row     index of LED row (0<=row<NUMROWS)
// Output: none
#define PWR_STRIPE0     10   // number of LEDs in first stripe
#define PWR_STRIPE0_RED 0
#define PWR_STRIPE0_GRN 255
#define PWR_STRIPE0_BLU 0
#define PWR_STRIPE1     10   // number of LEDs in second stripe
#define PWR_STRIPE1_RED 255
#define PWR_STRIPE1_GRN 255
#define PWR_STRIPE1_BLU 0
#define PWR_STRIPE2     10   // number of LEDs in third stripe
#define PWR_STRIPE2_RED 255
#define PWR_STRIPE2_GRN 128
#define PWR_STRIPE2_BLU 0
#define PWR_STRIPE3     225  // number of LEDs in fourth stripe (all remaining LEDs)
#define PWR_STRIPE3_RED 255
#define PWR_STRIPE3_GRN 0
#define PWR_STRIPE3_BLU 0
void Pattern_PwrMeterStriped(uint8_t litLEDs, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  WS2812_SetCursor(0, row);
  for(i=1; i<=NUMCOLS; i=i+1){
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(i < (litLEDs - 1)){  // LEDs near beginning of strand slightly on
      if(i <= PWR_STRIPE0){
        WS2812_AddColor(PWR_STRIPE0_RED>>3, PWR_STRIPE0_GRN>>3, PWR_STRIPE0_BLU>>3, row);
      } else if(i <= (PWR_STRIPE0 + PWR_STRIPE1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>3, PWR_STRIPE1_GRN>>3, PWR_STRIPE1_BLU>>3, row);
      } else if(i <= (PWR_STRIPE0 + PWR_STRIPE1 + PWR_STRIPE2)){
        WS2812_AddColor(PWR_STRIPE2_RED>>3, PWR_STRIPE2_GRN>>3, PWR_STRIPE2_BLU>>3, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED>>3, PWR_STRIPE3_GRN>>3, PWR_STRIPE3_BLU>>3, row);
      }
    } else if(i == (litLEDs - 1)){ // LED near end of bar slightly brighter
      if(i <= PWR_STRIPE0){
        WS2812_AddColor(PWR_STRIPE0_RED>>1, PWR_STRIPE0_GRN>>1, PWR_STRIPE0_BLU>>1, row);
      } else if(i <= (PWR_STRIPE0 + PWR_STRIPE1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>1, PWR_STRIPE1_GRN>>1, PWR_STRIPE1_BLU>>1, row);
      } else if(i <= (PWR_STRIPE0 + PWR_STRIPE1 + PWR_STRIPE2)){
        WS2812_AddColor(PWR_STRIPE2_RED>>1, PWR_STRIPE2_GRN>>1, PWR_STRIPE2_BLU>>1, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED>>1, PWR_STRIPE3_GRN>>1, PWR_STRIPE3_BLU>>1, row);
      }
    } else if(i == litLEDs){       // LED at end of bar full brightness
      if(i <= PWR_STRIPE0){
        WS2812_AddColor(PWR_STRIPE0_RED, PWR_STRIPE0_GRN, PWR_STRIPE0_BLU, row);
      } else if(i <= (PWR_STRIPE0 + PWR_STRIPE1)){
        WS2812_AddColor(PWR_STRIPE1_RED, PWR_STRIPE1_GRN, PWR_STRIPE1_BLU, row);
      } else if(i <= (PWR_STRIPE0 + PWR_STRIPE1 + PWR_STRIPE2)){
        WS2812_AddColor(PWR_STRIPE2_RED, PWR_STRIPE2_GRN, PWR_STRIPE2_BLU, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED, PWR_STRIPE3_GRN, PWR_STRIPE3_BLU, row);
      }
    } else{                        // LEDs beyond end of bar fully off
      WS2812_AddColor(0, 0, 0, row);
    }
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_PwrMeterColored------------
// Implement a power meter by illuminating a given number of
// LEDs, starting at the beginning of the strand.  The meter
// will be all one color, according to the number of LEDs
// illuminated.  A fully illuminated colored power meter will be
// fully red.  A partially illuminated colored power meter will
// be fully orange or fully yellow.
// Input: litLEDs number of LEDs to illuminate (litLEDs<=NUMCOLS)
//        row     index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_PwrMeterColored(uint8_t litLEDs, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  WS2812_SetCursor(0, row);
  for(i=1; i<=NUMCOLS; i=i+1){
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(litLEDs <= PWR_STRIPE0){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE0_RED>>3, PWR_STRIPE0_GRN>>3, PWR_STRIPE0_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE0_RED>>1, PWR_STRIPE0_GRN>>1, PWR_STRIPE0_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE0_RED, PWR_STRIPE0_GRN, PWR_STRIPE0_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else if(litLEDs <= (PWR_STRIPE0 + PWR_STRIPE1)){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>3, PWR_STRIPE1_GRN>>3, PWR_STRIPE1_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>1, PWR_STRIPE1_GRN>>1, PWR_STRIPE1_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE1_RED, PWR_STRIPE1_GRN, PWR_STRIPE1_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else if(litLEDs <= (PWR_STRIPE0 + PWR_STRIPE1 + PWR_STRIPE2)){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE2_RED>>3, PWR_STRIPE2_GRN>>3, PWR_STRIPE2_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE2_RED>>1, PWR_STRIPE2_GRN>>1, PWR_STRIPE2_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE2_RED, PWR_STRIPE2_GRN, PWR_STRIPE2_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else{
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE3_RED>>3, PWR_STRIPE3_GRN>>3, PWR_STRIPE3_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE3_RED>>1, PWR_STRIPE3_GRN>>1, PWR_STRIPE3_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE3_RED, PWR_STRIPE3_GRN, PWR_STRIPE3_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    }
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_SplitMeterBasic------------
// Implement a power meter by illuminating a given number of
// LEDs, starting in the middle of the strand.  The whole meter
// will always be the same constant color.
// Input: litLEDs     number of LEDs to illuminate (litLEDs<=LEDsPerSide<=(NUMCOLS/2))
//        LEDsPerSide number of LEDs on each side of the meter
//        row         index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_SplitMeterBasic(uint8_t litLEDs, uint8_t LEDsPerSide, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  if(LEDsPerSide > (NUMCOLS/2)){   // check that the whole meter fits in the LEDs
    LEDsPerSide = NUMCOLS/2;       // balance the number of LEDs per side if needed
  }
  if(litLEDs > LEDsPerSide){       // check that each half of the meter fits
    litLEDs = LEDsPerSide;         // light all of them
  }
  WS2812_SetCursor(0, row);
  for(i=LEDsPerSide; i>=1; i=i-1){ // draw first half of meter
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(i < (litLEDs - 1)){  // LEDs near middle of strand slightly on
      WS2812_AddColor(PWR_BASIC_RED>>3, PWR_BASIC_GRN>>3, PWR_BASIC_BLU>>3, row);
    } else if(i == (litLEDs - 1)){ // LED near end of bar slightly brighter
      WS2812_AddColor(PWR_BASIC_RED>>1, PWR_BASIC_GRN>>1, PWR_BASIC_BLU>>1, row);
    } else if(i == litLEDs){       // LED at end of bar full brightness
      WS2812_AddColor(PWR_BASIC_RED, PWR_BASIC_GRN, PWR_BASIC_BLU, row);
    } else{                        // LEDs beyond end of bar fully off
      WS2812_AddColor(0, 0, 0, row);
    }
  }
  for(i=1; i<=LEDsPerSide; i=i+1){ // draw second half of meter
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(i < (litLEDs - 1)){  // LEDs near middle of strand slightly on
      WS2812_AddColor(PWR_BASIC_RED>>3, PWR_BASIC_GRN>>3, PWR_BASIC_BLU>>3, row);
    } else if(i == (litLEDs - 1)){ // LED near end of bar slightly brighter
      WS2812_AddColor(PWR_BASIC_RED>>1, PWR_BASIC_GRN>>1, PWR_BASIC_BLU>>1, row);
    } else if(i == litLEDs){       // LED at end of bar full brightness
      WS2812_AddColor(PWR_BASIC_RED, PWR_BASIC_GRN, PWR_BASIC_BLU, row);
    } else{                        // LEDs beyond end of bar fully off
      WS2812_AddColor(0, 0, 0, row);
    }
  }
                                   // shut off any remaining LEDs
  for(i=(2*LEDsPerSide); i<NUMCOLS; i=i+1){
    WS2812_AddColor(0, 0, 0, row);
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_SplitMeterStriped------------
// Implement a power meter by illuminating a given number of
// LEDs, starting in the middle of the strand.  The meter will
// be striped, starting in the middle of the strand.
// A fully illuminated striped power meter will be made of red
// LEDs followed by orange, followed by yellow, followed by
// two bands of green, followed by yellow, followed by orange,
// followed by red.
// A partially illuminated striped power meter will be made of
// green LEDs surrounded by yellow, possibly also surrounded by
// orange.
// Input: litLEDs number of LEDs to illuminate (litLEDs<=LEDsPerSide<=(NUMCOLS/2))
//        LEDsPerSide number of LEDs on each side of the meter
//        row     index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_SplitMeterStriped(uint8_t litLEDs, uint8_t LEDsPerSide, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  if(LEDsPerSide > (NUMCOLS/2)){   // check that the whole meter fits in the LEDs
    LEDsPerSide = NUMCOLS/2;       // balance the number of LEDs per side if needed
  }
  if(litLEDs > LEDsPerSide){       // check that each half of the meter fits
    litLEDs = LEDsPerSide;         // light all of them
  }
  WS2812_SetCursor(0, row);
  for(i=LEDsPerSide; i>=1; i=i-1){ // draw first half of meter
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(i < (litLEDs - 1)){  // LEDs near middle of strand slightly on
      if(i <= (PWR_STRIPE0/2)){
        WS2812_AddColor(PWR_STRIPE0_RED>>3, PWR_STRIPE0_GRN>>3, PWR_STRIPE0_BLU>>3, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
        WS2812_AddColor(PWR_STRIPE1_RED>>3, PWR_STRIPE1_GRN>>3, PWR_STRIPE1_BLU>>3, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
        WS2812_AddColor(PWR_STRIPE2_RED>>3, PWR_STRIPE2_GRN>>3, PWR_STRIPE2_BLU>>3, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED>>3, PWR_STRIPE3_GRN>>3, PWR_STRIPE3_BLU>>3, row);
      }
    } else if(i == (litLEDs - 1)){ // LED near end of bar slightly brighter
      if(i <= (PWR_STRIPE0/2)){
        WS2812_AddColor(PWR_STRIPE0_RED>>1, PWR_STRIPE0_GRN>>1, PWR_STRIPE0_BLU>>1, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
        WS2812_AddColor(PWR_STRIPE1_RED>>1, PWR_STRIPE1_GRN>>1, PWR_STRIPE1_BLU>>1, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
        WS2812_AddColor(PWR_STRIPE2_RED>>1, PWR_STRIPE2_GRN>>1, PWR_STRIPE2_BLU>>1, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED>>1, PWR_STRIPE3_GRN>>1, PWR_STRIPE3_BLU>>1, row);
      }
    } else if(i == litLEDs){       // LED at end of bar full brightness
      if(i <= (PWR_STRIPE0/2)){
        WS2812_AddColor(PWR_STRIPE0_RED, PWR_STRIPE0_GRN, PWR_STRIPE0_BLU, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
        WS2812_AddColor(PWR_STRIPE1_RED, PWR_STRIPE1_GRN, PWR_STRIPE1_BLU, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
        WS2812_AddColor(PWR_STRIPE2_RED, PWR_STRIPE2_GRN, PWR_STRIPE2_BLU, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED, PWR_STRIPE3_GRN, PWR_STRIPE3_BLU, row);
      }
    } else{                        // LEDs beyond end of bar fully off
      WS2812_AddColor(0, 0, 0, row);
    }
  }
  for(i=1; i<=LEDsPerSide; i=i+1){ // draw second half of meter
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(i < (litLEDs - 1)){  // LEDs near middle of strand slightly on
      if(i <= (PWR_STRIPE0/2)){
        WS2812_AddColor(PWR_STRIPE0_RED>>3, PWR_STRIPE0_GRN>>3, PWR_STRIPE0_BLU>>3, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
        WS2812_AddColor(PWR_STRIPE1_RED>>3, PWR_STRIPE1_GRN>>3, PWR_STRIPE1_BLU>>3, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
        WS2812_AddColor(PWR_STRIPE2_RED>>3, PWR_STRIPE2_GRN>>3, PWR_STRIPE2_BLU>>3, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED>>3, PWR_STRIPE3_GRN>>3, PWR_STRIPE3_BLU>>3, row);
      }
    } else if(i == (litLEDs - 1)){ // LED near end of bar slightly brighter
      if(i <= (PWR_STRIPE0/2)){
        WS2812_AddColor(PWR_STRIPE0_RED>>1, PWR_STRIPE0_GRN>>1, PWR_STRIPE0_BLU>>1, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
        WS2812_AddColor(PWR_STRIPE1_RED>>1, PWR_STRIPE1_GRN>>1, PWR_STRIPE1_BLU>>1, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
        WS2812_AddColor(PWR_STRIPE2_RED>>1, PWR_STRIPE2_GRN>>1, PWR_STRIPE2_BLU>>1, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED>>1, PWR_STRIPE3_GRN>>1, PWR_STRIPE3_BLU>>1, row);
      }
    } else if(i == litLEDs){       // LED at end of bar full brightness
      if(i <= (PWR_STRIPE0/2)){
        WS2812_AddColor(PWR_STRIPE0_RED, PWR_STRIPE0_GRN, PWR_STRIPE0_BLU, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
        WS2812_AddColor(PWR_STRIPE1_RED, PWR_STRIPE1_GRN, PWR_STRIPE1_BLU, row);
      } else if(i <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
        WS2812_AddColor(PWR_STRIPE2_RED, PWR_STRIPE2_GRN, PWR_STRIPE2_BLU, row);
      } else{
        WS2812_AddColor(PWR_STRIPE3_RED, PWR_STRIPE3_GRN, PWR_STRIPE3_BLU, row);
      }
    } else{                        // LEDs beyond end of bar fully off
      WS2812_AddColor(0, 0, 0, row);
    }
  }
                                   // shut off any remaining LEDs
  for(i=(2*LEDsPerSide); i<NUMCOLS; i=i+1){
    WS2812_AddColor(0, 0, 0, row);
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}

//------------Pattern_SplitMeterColored------------
// Implement a power meter by illuminating a given number of
// LEDs, starting in the middle of the strand.  The meter will
// be all one color, according to the number of LEDs
// illuminated.  A fully illuminated colored power meter will be
// fully red.  A partially illuminated colored power meter will
// be fully orange or fully yellow.
// Input: litLEDs number of LEDs to illuminate (litLEDs<=LEDsPerSide<=(NUMCOLS/2))
//        LEDsPerSide number of LEDs on each side of the meter
//        row     index of LED row (0<=row<NUMROWS)
// Output: none
void Pattern_SplitMeterColored(uint8_t litLEDs, uint8_t LEDsPerSide, uint8_t row){
  uint32_t i;
  if(row >= NUMROWS){
    return;                        // row parameter is invalid
  }
  if(LEDsPerSide > (NUMCOLS/2)){   // check that the whole meter fits in the LEDs
    LEDsPerSide = NUMCOLS/2;       // balance the number of LEDs per side if needed
  }
  if(litLEDs > LEDsPerSide){       // check that each half of the meter fits
    litLEDs = LEDsPerSide;         // light all of them
  }
  WS2812_SetCursor(0, row);
  for(i=LEDsPerSide; i>=1; i=i-1){ // draw first half of meter
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(litLEDs <= (PWR_STRIPE0/2)){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE0_RED>>3, PWR_STRIPE0_GRN>>3, PWR_STRIPE0_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE0_RED>>1, PWR_STRIPE0_GRN>>1, PWR_STRIPE0_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE0_RED, PWR_STRIPE0_GRN, PWR_STRIPE0_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else if(litLEDs <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>3, PWR_STRIPE1_GRN>>3, PWR_STRIPE1_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>1, PWR_STRIPE1_GRN>>1, PWR_STRIPE1_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE1_RED, PWR_STRIPE1_GRN, PWR_STRIPE1_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else if(litLEDs <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE2_RED>>3, PWR_STRIPE2_GRN>>3, PWR_STRIPE2_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE2_RED>>1, PWR_STRIPE2_GRN>>1, PWR_STRIPE2_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE2_RED, PWR_STRIPE2_GRN, PWR_STRIPE2_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else{
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE3_RED>>3, PWR_STRIPE3_GRN>>3, PWR_STRIPE3_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE3_RED>>1, PWR_STRIPE3_GRN>>1, PWR_STRIPE3_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE3_RED, PWR_STRIPE3_GRN, PWR_STRIPE3_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    }
  }
  for(i=1; i<=LEDsPerSide; i=i+1){ // draw second half of meter
    if(litLEDs == 0){              // nothing to display; no LEDs on
      WS2812_AddColor(0, 0, 0, row);
    } else if(litLEDs <= (PWR_STRIPE0/2)){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE0_RED>>3, PWR_STRIPE0_GRN>>3, PWR_STRIPE0_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE0_RED>>1, PWR_STRIPE0_GRN>>1, PWR_STRIPE0_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE0_RED, PWR_STRIPE0_GRN, PWR_STRIPE0_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else if(litLEDs <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2))){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>3, PWR_STRIPE1_GRN>>3, PWR_STRIPE1_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE1_RED>>1, PWR_STRIPE1_GRN>>1, PWR_STRIPE1_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE1_RED, PWR_STRIPE1_GRN, PWR_STRIPE1_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else if(litLEDs <= ((PWR_STRIPE0/2) + (PWR_STRIPE1/2) + (PWR_STRIPE2/2))){
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE2_RED>>3, PWR_STRIPE2_GRN>>3, PWR_STRIPE2_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE2_RED>>1, PWR_STRIPE2_GRN>>1, PWR_STRIPE2_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE2_RED, PWR_STRIPE2_GRN, PWR_STRIPE2_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    } else{
      if(i < (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE3_RED>>3, PWR_STRIPE3_GRN>>3, PWR_STRIPE3_BLU>>3, row);
      } else if(i == (litLEDs - 1)){
        WS2812_AddColor(PWR_STRIPE3_RED>>1, PWR_STRIPE3_GRN>>1, PWR_STRIPE3_BLU>>1, row);
      } else if(i == litLEDs){
        WS2812_AddColor(PWR_STRIPE3_RED, PWR_STRIPE3_GRN, PWR_STRIPE3_BLU, row);
      } else{
        WS2812_AddColor(0, 0, 0, row);
      }
    }
  }
                                   // shut off any remaining LEDs
  for(i=(2*LEDsPerSide); i<NUMCOLS; i=i+1){
    WS2812_AddColor(0, 0, 0, row);
  }
  WS2812_SendReset();
  WS2812_PrintBuffer();
}
