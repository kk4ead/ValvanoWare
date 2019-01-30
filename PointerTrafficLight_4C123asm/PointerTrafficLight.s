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

        IMPORT   PLL_Init
        IMPORT   SysTick_Init
        IMPORT   SysTick_Wait10ms

LIGHT              EQU 0x400050FC   ; port B bits 5-0
GPIO_PORTB_DIR_R   EQU 0x40005400
GPIO_PORTB_AFSEL_R EQU 0x40005420
GPIO_PORTB_DEN_R   EQU 0x4000551C
GPIO_PORTB_AMSEL_R EQU 0x40005528
GPIO_PORTB_PCTL_R  EQU 0x4000552C
SENSOR             EQU 0x4002400C   ; port E bits 1-0
GPIO_PORTE_DIR_R   EQU 0x40024400
GPIO_PORTE_AFSEL_R EQU 0x40024420
GPIO_PORTE_DEN_R   EQU 0x4002451C
GPIO_PORTE_AMSEL_R EQU 0x40024528
GPIO_PORTE_PCTL_R  EQU 0x4002452C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
SYSCTL_RCGC2_GPIOE EQU 0x00000010   ; port E Clock Gating Control
SYSCTL_RCGC2_GPIOB EQU 0x00000002   ; port B Clock Gating Control

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
;Linked data structure
;Put in ROM
OUT   EQU 0    ;offset for output
WAIT  EQU 4    ;offset for time
NEXT  EQU 8    ;offset for next
goN   DCD 0x21 ;North green, East red
      DCD 3000 ;30 sec
      DCD goN,waitN,goN,waitN
waitN DCD 0x22 ;North yellow, East red
      DCD 500  ;5 sec
      DCD goE,goE,goE,goE
goE   DCD 0x0C ;North red, East green
      DCD 3000 ;30 sec
      DCD goE,goE,waitE,waitE
waitE DCD 0x14 ;North red, East yellow
      DCD 500  ;5 sec
      DCD goN,goN,goN,goN

Start
    BL  PLL_Init       ; 80 MHz clock
    BL  SysTick_Init   ; enable SysTick

    LDR R1, =SYSCTL_RCGCGPIO_R
    LDR R0, [R1]
    ORR R0, R0, #(SYSCTL_RCGC2_GPIOB+SYSCTL_RCGC2_GPIOE) ; activate port B and port E
    STR R0, [R1]
    NOP
    NOP                ; allow time to finish activating
    LDR R1, =GPIO_PORTB_PCTL_R
    LDR R0, [R1]
    BIC R0, R0, #0x00FFFFFF ; clear bits
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_AMSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x3F  ; no analog PB5-0
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_DIR_R
    LDR R0, [R1]
    ORR R0, R0, #0x3F  ; PB5-0 output
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_AFSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x3F  ; no alt funct
    STR R0, [R1]
    LDR R1, =GPIO_PORTB_DEN_R
    LDR R0, [R1]
    ORR R0, R0, #0x3F  ; enable PB5-0
    STR R0, [R1]

    LDR R1, =GPIO_PORTE_PCTL_R
    LDR R0, [R1]
    BIC R0, R0, #0x000000FF ; clear bits
    STR R0, [R1]
    LDR R1, =GPIO_PORTE_AMSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x03  ; no analog PE1-0
    STR R0, [R1]
    LDR R1, =GPIO_PORTE_DIR_R
    LDR R0, [R1]
    BIC R0, R0, #0x03  ; PE1-0 input
    STR R0, [R1]
    LDR R1, =GPIO_PORTE_AFSEL_R
    LDR R0, [R1]
    BIC R0, R0, #0x03  ; no alt funct
    STR R0, [R1]
    LDR R1, =GPIO_PORTE_DEN_R
    LDR R0, [R1]
    ORR R0, R0, #0x03  ; enable PE1-0
    STR R0, [R1]

    LDR R4, =goN       ; state pointer
    LDR R5, =SENSOR    ; 0x4002400C
    LDR R6, =LIGHT     ; 0x400050FC
FSM LDR R0, [R4, #OUT] ; output value
    STR R0, [R6]       ; set lights
    LDR R0, [R4, #WAIT]; time delay
    BL  SysTick_Wait10ms
    LDR R0, [R5]       ; read input
    LSL R0, R0, #2     ; 4 bytes/address
    ADD R0, R0, #NEXT  ; 8,12,16,20
    LDR R4, [R4, R0]   ; go to next state
    B   FSM

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
