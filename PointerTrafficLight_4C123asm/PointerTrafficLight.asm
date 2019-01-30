; PointerTrafficLight.s
; Runs on LM4F120/TM4C123
; Use a pointer implementation of a Moore finite state machine to operate
; a traffic light.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 6.8, Example 6.4
;  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;  ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;  Program 3.1, Example 3.1
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

; east facing red light connected to PB5
; east facing yellow light connected to PB4
; east facing green light connected to PB3
; north facing red light connected to PB2
; north facing yellow light connected to PB1
; north facing green light connected to PB0
; north facing car detector connected to PE1 (1=car present)
; east facing car detector connected to PE0 (1=car present)
        .thumb
        .text
        .align 2
        .global   PLL_Init
        .global   SysTick_Init
        .global   SysTick_Wait10ms

LIGHT              .field 0x400050FC,32   ; port B bits 5-0
GPIO_PORTB_DIR_R   .field 0x40005400,32
GPIO_PORTB_AFSEL_R .field 0x40005420,32
GPIO_PORTB_DEN_R   .field 0x4000551C,32
GPIO_PORTB_AMSEL_R .field 0x40005528,32
GPIO_PORTB_PCTL_R  .field 0x4000552C,32
SENSOR             .field 0x4002400C,32   ; port E bits 1-0
GPIO_PORTE_DIR_R   .field 0x40024400,32
GPIO_PORTE_AFSEL_R .field 0x40024420,32
GPIO_PORTE_DEN_R   .field 0x4002451C,32
GPIO_PORTE_AMSEL_R .field 0x40024528,32
GPIO_PORTE_PCTL_R  .field 0x4002452C,32
SYSCTL_RCGCGPIO_R  .field 0x400FE608,32
SYSCTL_RCGC2_GPIOE .equ 0x00000010   ; port E Clock Gating Control
SYSCTL_RCGC2_GPIOB .equ 0x00000002   ; port B Clock Gating Control


        .global  main
;Linked data structure
;Put in ROM
OUT   .equ 0    ;offset for output
WAIT  .equ 4    ;offset for time
NEXT  .equ 8    ;offset for next
goN   .long 0x21 ;North green, East red
      .long 3000 ;30 sec
      .long goN,waitN,goN,waitN
waitN .long 0x22 ;North yellow, East red
      .long 500  ;5 sec
      .long goE,goE,goE,goE
goE   .long 0x0C ;North red, East green
      .long 3000 ;30 sec
      .long goE,goE,waitE,waitE
waitE .long 0x14 ;North red, East yellow
      .long 500  ;5 sec
      .long goN,goN,goN,goN
goNAddr .field goN,32
main:  .asmfunc
    BL  PLL_Init       ; 80 MHz clock
    BL  SysTick_Init   ; enable SysTick

    LDR R1, SYSCTL_RCGCGPIO_R
    LDR R0, [R1]
    ORR R0, R0, #(SYSCTL_RCGC2_GPIOB+SYSCTL_RCGC2_GPIOE) ; activate port B and port E
    STR R0, [R1]
    NOP
    NOP                ; allow time to finish activating
    LDR R1, GPIO_PORTB_PCTL_R
    LDR R0, [R1]
    AND R0, R0, #0xFF000000 ; clear bits
    STR R0, [R1]
    LDR R1, GPIO_PORTB_AMSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x3F  ; no analog PB5-0
    STR R0, [R1]
    LDR R1, GPIO_PORTB_DIR_R
    LDR R0, [R1]
    ORR R0, R0, #0x3F  ; PB5-0 output
    STR R0, [R1]
    LDR R1, GPIO_PORTB_AFSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x3F  ; no alt funct
    STR R0, [R1]
    LDR R1, GPIO_PORTB_DEN_R
    LDR R0, [R1]
    ORR R0, R0, #0x3F  ; enable PB5-0
    STR R0, [R1]

    LDR R1, GPIO_PORTE_PCTL_R
    LDR R0, [R1]
    BIC R0, R0, #0x000000FF ; clear bits
    STR R0, [R1]
    LDR R1, GPIO_PORTE_AMSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x03  ; no analog PE1-0
    STR R0, [R1]
    LDR R1, GPIO_PORTE_DIR_R
    LDR R0, [R1]
    BIC R0, R0, #0x03  ; PE1-0 input
    STR R0, [R1]
    LDR R1, GPIO_PORTE_AFSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x03  ; no alt funct
    STR R0, [R1]
    LDR R1, GPIO_PORTE_DEN_R
    LDR R0, [R1]
    ORR R0, R0, #0x03  ; enable PE1-0
    STR R0, [R1]

    LDR R4, goNAddr   ; state pointer
    LDR R5, SENSOR    ; 0x4002400C
    LDR R6, LIGHT     ; 0x400050FC
FSM LDR R0, [R4, #OUT] ; output value
    STR R0, [R6]       ; set lights
    LDR R0, [R4, #WAIT]; time delay
    BL  SysTick_Wait10ms
    LDR R0, [R5]       ; read input
    LSL R0, R0, #2     ; 4 bytes/address
    ADD R0, R0, #NEXT  ; 8,12,16,20
    LDR R4, [R4, R0]   ; go to next state
    B   FSM
    .endasmfunc
    .end                             ; end of file
