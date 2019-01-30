; EngineControl.s
; Runs on LM4F120/TM4C123
; Use a pointer implementation of a Mealy finite state machine to operate
; an engine with a control input, gas and brake outputs, and two states.
; Daniel Valvano
; September 11, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 6.9
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

; control input connected to PB0 (1=go)
; gas output connected to PB1
; brake output connected to PB2

        IMPORT   PLL_Init
        IMPORT   SysTick_Init
        IMPORT   SysTick_Wait10ms

INPUT              EQU 0x40005004   ; port B bit 0
OUTPUT             EQU 0x40005018   ; port B bits 2-1
GPIO_PORTB_DIR_R   EQU 0x40005400
GPIO_PORTB_AFSEL_R EQU 0x40005420
GPIO_PORTB_DEN_R   EQU 0x4000551C
GPIO_PORTB_AMSEL_R EQU 0x40005528
GPIO_PORTB_PCTL_R  EQU 0x4000552C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
;Linked data structure
;Put in ROM
OUT   EQU 0    ;offset for output
DELAY EQU 8    ;offset for delay between states
NEXT  EQU 12   ;offset for next
Stop  DCD 2,0     ;Outputs for 0,1
      DCD 10      ;100 ms
      DCD Stop,Go ;Next for 0,1
Go    DCD 0,1     ;Outputs for 0,1
      DCD 10      ;100 ms
      DCD Stop,Go ;Next for 0,1

Start
    BL  PLL_Init      ; 50 MHz clock
    BL  SysTick_Init  ; enable SysTick
    LDR R1, =SYSCTL_RCGCGPIO_R
    LDR R0, [R1]
    ORR R0, R0, #0x02 ; activate port B
    STR R0, [R1]
    NOP
    NOP               ; allow time to finish activating
    LDR R1, =GPIO_PORTB_PCTL_R
    LDR R0, [R1]
    MOV R2, #0x00000FFF
    BIC R0, R0, R2          ; clear port control field for PA2-0
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_AMSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x07 ; disable analog functionality on PA2-0
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_DIR_R
    LDR R0, [R1]
    BIC R0, R0, #0x01 ; PA0 input
    ORR R0, R0, #0x06 ; PA2-1 output
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_AFSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x07 ; no alt funct
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_DEN_R
    LDR R0, [R1]
    ORR R0, R0, #0x07 ; enable PA2-0
    STR R0, [R1]

    LDR R4, =Stop     ; State pointer
    LDR R5, =INPUT    ; 0x40005004
    LDR R6, =OUTPUT   ; 0x40005018
FSM LDR R3, [R5]      ; Read input
    LSL R3, R3, #2    ; 4 bytes each
    ADD R1, R3, #OUT  ; R1 is 0 or 4
    LDR R2, [R4, R1]  ; Output value
    LSL R2, R2, #1    ; into bits 2,1
    STR R2, [R6]      ; set outputs
    LDR R0, [R4, #DELAY] ; num 10ms delays
    ADD R1, R3, #NEXT ; R1 is 12 or 16
    LDR R4, [R4, R1]  ; find next state
    BL  SysTick_Wait10ms
    B   FSM           ; go to next state

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
