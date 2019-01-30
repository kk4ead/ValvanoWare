// ProfileFFT.c
// Runs on LM4F120/TM4C123
// Daniel Valvano
// February 11, 2015

/* This example accompanies the books
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Volume 3, Programs 3.14-3.16

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

// PF2 is an output for debugging
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

typedef struct{
  int16_t real,imag;
}Complex_t;
// data for FFT
Complex_t x[1024],y[1024]; // input and output arrays for FFT
// two 16-bit signed numbers are packed into each 32-bit entry
// bits31-16 are imaginary part, signed 16 bits
// bits15-0 are real part, signed 16 bits
// for the input, normally x[] bits31-16 are cleared to mean real input
//                         x[] bits15-0 contain the input signal
// for the output,         y[] bits31-16 contain the imaginary part
//                         y[] bits15-0 contain the real part
int32_t mag[512];
//*********Prototype for FFT in cr4_fft_1024_stm32.s, STMicroelectronics
void cr4_fft_1024_stm32(Complex_t *pssOUT, Complex_t *pssIN, unsigned short Nbin);
void cr4_fft_256_stm32(Complex_t *pssOUT, Complex_t *pssIN, unsigned short Nbin);
void cr4_fft_64_stm32(Complex_t *pssOUT, Complex_t *pssIN, unsigned short Nbin);


#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value
#define PF2   (*((volatile uint32_t *)0x40025010))
#define Debug_Set()   (PF2 = 0x04)
#define Debug_Clear() (PF2 = 0x00)

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}
// Newton's method
// s is an integer
// Sqrt(s) is an integer
uint32_t Sqrt(uint32_t s){
uint32_t t;   // t*t will become s
int n;             // loop counter
  t = s/16+1;      // initial guess
  for(n = 20; n; --n){ // will finish
    t = ((t*t+s)/t)/2;
  }
  return t;
}
uint32_t before,elapsed64,elapsed256,elapsed1024;
// 1024 + 1024*sin(2*pi*2*t); (2 waves per 1024 samples)
const int16_t sinewave[1024] = {
  1024, 1037, 1049, 1062, 1074, 1087, 1099, 1112, 1124, 1137, 1149, 1162, 1174, 1187, 1199, 1211,
  1224, 1236, 1248, 1261, 1273, 1285, 1297, 1309, 1321, 1333, 1345, 1357, 1369, 1381, 1393, 1404,
  1416, 1427, 1439, 1450, 1462, 1473, 1484, 1496, 1507, 1518, 1529, 1540, 1550, 1561, 1572, 1582,
  1593, 1603, 1614, 1624, 1634, 1644, 1654, 1664, 1674, 1683, 1693, 1702, 1712, 1721, 1730, 1739,
  1748, 1757, 1766, 1774, 1783, 1791, 1799, 1808, 1816, 1823, 1831, 1839, 1846, 1854, 1861, 1868,
  1875, 1882, 1889, 1896, 1902, 1909, 1915, 1921, 1927, 1933, 1939, 1944, 1950, 1955, 1960, 1965,
  1970, 1975, 1979, 1984, 1988, 1992, 1996, 2000, 2004, 2007, 2011, 2014, 2017, 2020, 2023, 2026,
  2028, 2031, 2033, 2035, 2037, 2039, 2040, 2042, 2043, 2044, 2045, 2046, 2047, 2047, 2048, 2048,
  2048, 2048, 2048, 2047, 2047, 2046, 2045, 2044, 2043, 2042, 2040, 2039, 2037, 2035, 2033, 2031,
  2028, 2026, 2023, 2020, 2017, 2014, 2011, 2007, 2004, 2000, 1996, 1992, 1988, 1984, 1979, 1975,
  1970, 1965, 1960, 1955, 1950, 1944, 1939, 1933, 1927, 1921, 1915, 1909, 1902, 1896, 1889, 1882,
  1875, 1868, 1861, 1854, 1846, 1839, 1831, 1823, 1816, 1808, 1799, 1791, 1783, 1774, 1766, 1757,
  1748, 1739, 1730, 1721, 1712, 1702, 1693, 1683, 1674, 1664, 1654, 1644, 1634, 1624, 1614, 1603,
  1593, 1582, 1572, 1561, 1550, 1540, 1529, 1518, 1507, 1496, 1484, 1473, 1462, 1450, 1439, 1427,
  1416, 1404, 1393, 1381, 1369, 1357, 1345, 1333, 1321, 1309, 1297, 1285, 1273, 1261, 1248, 1236,
  1224, 1211, 1199, 1187, 1174, 1162, 1149, 1137, 1124, 1112, 1099, 1087, 1074, 1062, 1049, 1037,
  1024, 1011, 999, 986, 974, 961, 949, 936, 924, 911, 899, 886, 874, 861, 849, 837,
  824, 812, 800, 787, 775, 763, 751, 739, 727, 715, 703, 691, 679, 667, 655, 644,
  632, 621, 609, 598, 586, 575, 564, 552, 541, 530, 519, 508, 498, 487, 476, 466,
  455, 445, 434, 424, 414, 404, 394, 384, 374, 365, 355, 346, 336, 327, 318, 309,
  300, 291, 282, 274, 265, 257, 249, 240, 232, 225, 217, 209, 202, 194, 187, 180,
  173, 166, 159, 152, 146, 139, 133, 127, 121, 115, 109, 104, 98, 93, 88, 83,
  78, 73, 69, 64, 60, 56, 52, 48, 44, 41, 37, 34, 31, 28, 25, 22,
  20, 17, 15, 13, 11, 9, 8, 6, 5, 4, 3, 2, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 2, 3, 4, 5, 6, 8, 9, 11, 13, 15, 17,
  20, 22, 25, 28, 31, 34, 37, 41, 44, 48, 52, 56, 60, 64, 69, 73,
  78, 83, 88, 93, 98, 104, 109, 115, 121, 127, 133, 139, 146, 152, 159, 166,
  173, 180, 187, 194, 202, 209, 217, 225, 232, 240, 249, 257, 265, 274, 282, 291,
  300, 309, 318, 327, 336, 346, 355, 365, 374, 384, 394, 404, 414, 424, 434, 445,
  455, 466, 476, 487, 498, 508, 519, 530, 541, 552, 564, 575, 586, 598, 609, 621,
  632, 644, 655, 667, 679, 691, 703, 715, 727, 739, 751, 763, 775, 787, 800, 812,
  824, 837, 849, 861, 874, 886, 899, 911, 924, 936, 949, 961, 974, 986, 999, 1011,
  1024, 1037, 1049, 1062, 1074, 1087, 1099, 1112, 1124, 1137, 1149, 1162, 1174, 1187, 1199, 1211,
  1224, 1236, 1248, 1261, 1273, 1285, 1297, 1309, 1321, 1333, 1345, 1357, 1369, 1381, 1393, 1404,
  1416, 1427, 1439, 1450, 1462, 1473, 1484, 1496, 1507, 1518, 1529, 1540, 1550, 1561, 1572, 1582,
  1593, 1603, 1614, 1624, 1634, 1644, 1654, 1664, 1674, 1683, 1693, 1702, 1712, 1721, 1730, 1739,
  1748, 1757, 1766, 1774, 1783, 1791, 1799, 1808, 1816, 1823, 1831, 1839, 1846, 1854, 1861, 1868,
  1875, 1882, 1889, 1896, 1902, 1909, 1915, 1921, 1927, 1933, 1939, 1944, 1950, 1955, 1960, 1965,
  1970, 1975, 1979, 1984, 1988, 1992, 1996, 2000, 2004, 2007, 2011, 2014, 2017, 2020, 2023, 2026,
  2028, 2031, 2033, 2035, 2037, 2039, 2040, 2042, 2043, 2044, 2045, 2046, 2047, 2047, 2048, 2048,
  2048, 2048, 2048, 2047, 2047, 2046, 2045, 2044, 2043, 2042, 2040, 2039, 2037, 2035, 2033, 2031,
  2028, 2026, 2023, 2020, 2017, 2014, 2011, 2007, 2004, 2000, 1996, 1992, 1988, 1984, 1979, 1975,
  1970, 1965, 1960, 1955, 1950, 1944, 1939, 1933, 1927, 1921, 1915, 1909, 1902, 1896, 1889, 1882,
  1875, 1868, 1861, 1854, 1846, 1839, 1831, 1823, 1816, 1808, 1799, 1791, 1783, 1774, 1766, 1757,
  1748, 1739, 1730, 1721, 1712, 1702, 1693, 1683, 1674, 1664, 1654, 1644, 1634, 1624, 1614, 1603,
  1593, 1582, 1572, 1561, 1550, 1540, 1529, 1518, 1507, 1496, 1484, 1473, 1462, 1450, 1439, 1427,
  1416, 1404, 1393, 1381, 1369, 1357, 1345, 1333, 1321, 1309, 1297, 1285, 1273, 1261, 1248, 1236,
  1224, 1211, 1199, 1187, 1174, 1162, 1149, 1137, 1124, 1112, 1099, 1087, 1074, 1062, 1049, 1037,
  1024, 1011, 999, 986, 974, 961, 949, 936, 924, 911, 899, 886, 874, 861, 849, 837,
  824, 812, 800, 787, 775, 763, 751, 739, 727, 715, 703, 691, 679, 667, 655, 644,
  632, 621, 609, 598, 586, 575, 564, 552, 541, 530, 519, 508, 498, 487, 476, 466,
  455, 445, 434, 424, 414, 404, 394, 384, 374, 365, 355, 346, 336, 327, 318, 309,
  300, 291, 282, 274, 265, 257, 249, 240, 232, 225, 217, 209, 202, 194, 187, 180,
  173, 166, 159, 152, 146, 139, 133, 127, 121, 115, 109, 104, 98, 93, 88, 83,
  78, 73, 69, 64, 60, 56, 52, 48, 44, 41, 37, 34, 31, 28, 25, 22,
  20, 17, 15, 13, 11, 9, 8, 6, 5, 4, 3, 2, 1, 1, 0, 0,
  0, 0, 0, 1, 1, 2, 3, 4, 5, 6, 8, 9, 11, 13, 15, 17,
  20, 22, 25, 28, 31, 34, 37, 41, 44, 48, 52, 56, 60, 64, 69, 73,
  78, 83, 88, 93, 98, 104, 109, 115, 121, 127, 133, 139, 146, 152, 159, 166,
  173, 180, 187, 194, 202, 209, 217, 225, 232, 240, 249, 257, 265, 274, 282, 291,
  300, 309, 318, 327, 336, 346, 355, 365, 374, 384, 394, 404, 414, 424, 434, 445,
  455, 466, 476, 487, 498, 508, 519, 530, 541, 552, 564, 575, 586, 598, 609, 621,
  632, 644, 655, 667, 679, 691, 703, 715, 727, 739, 751, 763, 775, 787, 800, 812,
  824, 837, 849, 861, 874, 886, 899, 911, 924, 936, 949, 961, 974, 986, 999, 1011};

int main(void){
  int32_t t,k, real, imag;
  SYSCTL_RCGCGPIO_R |= 0x20; // activate Port F
  SysTick_Init();            // initialize SysTick timer, Program 4.7
  GPIO_PORTF_DIR_R |= 0x04;  // make PF2 out (built-in blue LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;// disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;  // enable digital I/O on PF2
                             // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;    // disable analog functionality on PF

// ****************64 element test************************
  for(t=0; t<64; t=t+1){     // t means 1/fs
    x[t].imag = 0;           // imaginary part is zero
    x[t].real = sinewave[t*16]; // fill real part with data
  }
  before = NVIC_ST_CURRENT_R;
  cr4_fft_64_stm32(y, x, 64);   // complex FFT of 64 values
  elapsed64 = (before - NVIC_ST_CURRENT_R - 6)&0x00FFFFFF;
  // the number 6 depends on the instructions before and after test
  // if you remove the call to FFT, elapsed measures 0
//*********************************************************

// ****************256 element test************************
  for(t=0; t<256; t=t+1){    // t means 1/fs
    x[t].imag = 0;           // imaginary part is zero
    x[t].real = sinewave[t*4]; // fill real part with data
  }
  before = NVIC_ST_CURRENT_R;
  cr4_fft_256_stm32(y, x, 256);   // complex FFT of 256 values
  elapsed256 = (before - NVIC_ST_CURRENT_R - 4)&0x00FFFFFF;
  // the number 4 depends on the instructions before and after test
  // if you remove the call to FFT, elapsed measures 0
//*********************************************************

// ****************1024 element test************************
  for(t=0; t<1024; t=t+1){   // t means 1/fs
    x[t].imag = 0;           // imaginary part is zero
    x[t].real = sinewave[t]; // fill real part with data
  }
  before = NVIC_ST_CURRENT_R;
  cr4_fft_1024_stm32(y, x, 1024);   // complex FFT of 1024 values
  elapsed1024 = (before - NVIC_ST_CURRENT_R - 4)&0x00FFFFFF;
  // the number 4 depends on the instructions before and after test
  // if you remove the call to FFT, elapsed measures 0
//*********************************************************


  while(1){
    for(t=0; t<1024; t=t+1){   // simulated ADC samples
      x[t].imag = 0;           // imaginary part is zero
      x[t].real = sinewave[t]; // fill real part with data
    }
    Debug_Set();                    // PF2=1
    cr4_fft_1024_stm32(y, x, 1024); // complex FFT of last 1024 ADC values
    Debug_Clear();                  // PF2=0
    for(k=0; k<512; k=k+1){         // k means fs/1024
      real = y[k].real;             // bottom 16 bits
      imag = y[k].imag;             // top 16 bits
      mag[k] = Sqrt(real*real+imag*imag);
    }
  }
  /*
  while(1){
    for(t=0; t<64; t=t+1){     // simulated ADC samples
      x[t].imag = 0;           // imaginary part is zero
      x[t].real = sinewave[t*16]; // fill real part with data
    }
    Debug_Set();                    // PF2=1
    cr4_fft_64_stm32(y, x, 64); // complex FFT of last 64 ADC values
    Debug_Clear();                  // PF2=0
    for(k=0; k<32; k=k+1){         // k means fs/64
      real = y[k].real;             // bottom 16 bits
      imag = y[k].imag;             // top 16 bits
      mag[k] = Sqrt(real*real+imag*imag);
    }
  }
  while(1){
    for(t=0; t<256; t=t+1){   // simulated ADC samples
      x[t].imag = 0;           // imaginary part is zero
      x[t].real = sinewave[t*4]; // fill real part with data
    }
    Debug_Set();                    // PF2=1
    cr4_fft_256_stm32(y, x, 256); // complex FFT of last 256 ADC values
    Debug_Clear();                  // PF2=0
    for(k=0; k<128; k=k+1){         // k means fs/256
      real = y[k].real;             // bottom 16 bits
      imag = y[k].imag;             // top 16 bits
      mag[k] = Sqrt(real*real+imag*imag);
    }
  }
  */
}
