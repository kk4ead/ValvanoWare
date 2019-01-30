; SysTickTestMain.s
; Runs on LM4F120/TM4C123
; Test the SysTick functions by activating the PLL, initializing the
; SysTick timer, and flashing an LED at a constant rate.
; Daniel Valvano
; September 12, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 4.6, Section 4.3
;  Program 4.7, Section 4.4
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

        .global   SysTick_Init
        .global   SysTick_Wait
        .global   SysTick_Wait10ms
        .global   PLL_Init

GPIO_PORTF2        .field 0x40025010,32
GPIO_PORTF_DIR_R   .field 0x40025400,32
GPIO_PORTF_AFSEL_R .field 0x40025420,32
GPIO_PORTF_DEN_R   .field 0x4002551C,32
GPIO_PORTF_AMSEL_R .field 0x40025528,32
GPIO_PORTF_PCTL_R  .field 0x4002552C,32
SYSCTL_RCGCGPIO_R  .field 0x400FE608,32


        .global  main
main:  .asmfunc
    BL  PLL_Init                    ; set system clock to 80 MHz
    BL  SysTick_Init                ; initialize SysTick timer
    ; activate clock for Port F
    LDR R1, SYSCTL_RCGCGPIO_R       ; R1 = &SYSCTL_RCGCGPIO_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x20               ; Port F
    STR R0, [R1]                    ; [R1] = R0
    NOP
    NOP                             ; allow time to finish activating
    ; set direction register
    LDR R1, GPIO_PORTF_DIR_R        ; R1 = &GPIO_PORTF_DIR_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x04               ; R0 = R0|0x04 (make PF2 output; PF2 built-in blue LED)
    STR R0, [R1]                    ; [R1] = R0
    ; regular port function
    LDR R1, GPIO_PORTF_AFSEL_R      ; R1 = &GPIO_PORTF_AFSEL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x04               ; R0 = R0&~0x04 (disable alt funct on PF2)
    STR R0, [R1]                    ; [R1] = R0
    ; enable digital port
    LDR R1, GPIO_PORTF_DEN_R        ; R1 = &GPIO_PORTF_DEN_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x04               ; R0 = R0|0x04 (enable digital I/O on PF2)
    STR R0, [R1]                    ; [R1] = R0
    ; configure as GPIO
    LDR R1, GPIO_PORTF_PCTL_R       ; R1 = &GPIO_PORTF_PCTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x00000F00         ; R0 = R0&~0x00000F00 (clear port control field for PF2)
    ADD R0, R0, #0x00000000         ; R0 = R0+0x00000000 (configure PF2 as GPIO)
    STR R0, [R1]                    ; [R1] = R0
    ; disable analog functionality
    LDR R1, GPIO_PORTF_AMSEL_R      ; R1 = &GPIO_PORTF_AMSEL_R
    MOV R0, #0                      ; R0 = 0 (disable analog functionality on PF)
    STR R0, [R1]                    ; [R1] = R0
    LDR R4, GPIO_PORTF2             ; R4 = &GPIO_PORTF2
    LDR R5, [R4]                    ; R5 = [R4]
loop
    EOR R5, R5, #0x04               ; R5 = R5^0x04 (toggle PF2)
    STR R5, [R4]                    ; [R4] = R5
;    MOV R0, #1                      ; approximately 720 ns
;    MOV R0, #2                      ; approximately 720 ns
;    MOV R0, #10000                  ; approximately 0.2 ms
;    BL  SysTick_Wait
    MOV R0, #1                      ; exactly 10 ms
    BL  SysTick_Wait10ms
    B   loop
    .endasmfunc

    .end                             ; end of file
