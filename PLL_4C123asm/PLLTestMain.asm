; PLLTestMain.asm
; Runs on LM4F120/TM4C123
; Test the PLL function to verify that the system clock is
; running at the expected rate.  Use the debugger if possible
; or an oscilloscope connected to PF2.
; The EQU statement allows the function PLL_Init() to
; initialize the PLL to the desired frequency.  When using an
; oscilloscope to look at LED1, it should be clear to see that
; the LED flashes about 2 (80/40) times faster with a 80 MHz
; clock than with a 40 MHz clock.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 4.6, Section 4.3
;  Volume 2 Program 2.10, Figure 2.37
;
;Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
;   You may use, edit, run or distribute this file
;   as long as the above copyright notice remains
;THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
;OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
;MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
;VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
;OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;For more information about my classes, my research, and my books, see
;http://users.ece.utexas.edu/~valvano/
   .thumb
   .text
   .align 2
GPIO_PORTF2        .field 0x40025010,32
GPIO_PORTF_DIR_R   .field 0x40025400,32
GPIO_PORTF_AFSEL_R .field 0x40025420,32
GPIO_PORTF_DEN_R   .field 0x4002551C,32
GPIO_PORTF_AMSEL_R .field 0x40025528,32
GPIO_PORTF_PCTL_R  .field 0x4002552C,32
SYSCTL_RCGCGPIO_R  .field 0x400FE608,32
;ONEHZDELAY         .field 8333333,32     ; delay ~0.5 sec at 50 MHz
ONEHZDELAY         .field 13333333,32    ; delay ~0.5 sec at 80 MHz


        .global  main
        .global PLL_Init

;------------Delay------------
; delay function for testing from sysctl.c
; which delays 3*R0 cycles
; Input: R0  number of cycles to wait divided by 3
; Output: none
; Modifies: R0
Delay:  .asmfunc
    subs    r0, #1
    bne     Delay
    bx      lr
    .endasmfunc

main:  .asmfunc
    BL  PLL_Init                    ; initialize PLL, 80 MHz
    ; activate clock for Port F
    LDR R1, SYSCTL_RCGCGPIO_R
    LDR R0, [R1]
    ORR R0, R0, #0x20               ; Port F clock
    STR R0, [R1]
    NOP
    NOP                             ; allow time to finish activating
    ; set direction register
    LDR R1, GPIO_PORTF_DIR_R
    LDR R0, [R1]
    ORR R0, R0, #0x04               ; make PF2 output
    STR R0, [R1]
    ; regular port function
    LDR R1, GPIO_PORTF_AFSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x04               ; disable alt funct on PF2
    STR R0, [R1]
    ; enable digital port
    LDR R1, GPIO_PORTF_DEN_R
    LDR R0, [R1]
    ORR R0, R0, #0x04               ; enable digital I/O on PF2
    STR R0, [R1]
    ; configure PF4 as GPIO
    LDR R1, GPIO_PORTF_PCTL_R
    LDR R0, [R1]
    BIC R0, R0, #0x000F0000         ; clear bit4 field for GPIO functionality)
    STR R0, [R1]
    ; disable analog functionality on PF
    LDR R1, GPIO_PORTF_AMSEL_R
    MOV R0, #0                      ; no analog pins on PF on this microcontroller
    STR R0, [R1]
    LDR R4, GPIO_PORTF2            ; R4 = GPIO_PORTF2
    MOV R5, #0x04                   ; R5 = 0x04 (LED on)
    MOV R6, #0x00                   ; R6 = 0x00 (LED off)
loop
    STR R5, [R4]                    ; [R4] = R5 = 0x04 (turn on LED1 (blue))
    LDR R0, ONEHZDELAY             ; R0 = ONEHZDELAY
    BL  Delay                       ; delay ~0.5 sec at 50 MHz
    STR R6, [R4]                    ; [R4] = R6 = 0x00 (turn off LED1 (blue))
    LDR R0, ONEHZDELAY             ; R0 = ONEHZDELAY
    BL  Delay                       ; delay ~0.5 sec at 50 MHz
    B   loop
    .endasmfunc

    .end                             ; end of file
