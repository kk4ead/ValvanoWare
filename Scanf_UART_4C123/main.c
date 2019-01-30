//**** 0. Documentation Section
//  This program calculates the area of square shaped rooms
//  Author: Ramesh Yerraballi & Jon Valvano
//  Date: 5/24/2014
//
// 1. Pre-processor Directives Section
#include <stdio.h>  // Diamond braces for sys lib: Standard I/O
#include <stdint.h> // C99 variable types
void Output_Init(void);

// 2. Global Declarations section
int32_t side; // room wall meters
int32_t area; // size squared meters
// Function Prototypes

// 3. Subroutines Section
// MAIN: Mandatory routine for a C program to be executable
int main(void) {
  Output_Init();              // initialize output device
  printf("This program calculates areas of square-shaped rooms\n");
  while(1){
    printf("Give room side:");  // 1) ask for input
    scanf("%ld", &side);        // 2) wait for input
    area = side*side;           // 3) calculation
    printf("\nside = %ld m, area = %ld sqr m\n", side, area); // 4) out
  }
}
