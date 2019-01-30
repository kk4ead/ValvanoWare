; PeriodicTimer0AInts.s
; Runs on LM4F120/TM4C123
; Use Timer0A in periodic mode to request interrupts at a particular
; period.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;   Volume 1, Program 9.8

;  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;   Volume 2, Program 7.5, example 7.6

; Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
;   You may use, edit, run or distribute this file
;   as long as the above copyright notice remains
;THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
;OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
;MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
;VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
;OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;For more information about my classes, my research, and my books, see
;http://users.ece.utexas.edu/~valvano/

; oscilloscope or LED connected to PF2 for period measurement

        IMPORT   PLL_Init
        IMPORT   Timer0A_Init

TIMER0_ICR_R       EQU 0x40030024
TIMER_ICR_TATOCINT EQU 0x00000001   ; GPTM TimerA Time-Out Raw
                                    ; Interrupt
GPIO_PORTF2        EQU 0x40025010
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Timer0A_Handler
        EXPORT  Start
Timer0A_Handler
    ; acknowledge timer0A timeout
    LDR R1, =TIMER0_ICR_R           
    MOV R0, #0x01              ;clear TATORIS flag, acknowledge interrupt
    STR R0, [R1]              
    ; toggle LED
    LDR R1, =GPIO_PORTF2     
    LDR R0, [R1]              
    EOR R0, R0, #0x04          ; toggle PF2
    STR R0, [R1]               
    BX  LR                     ; return from interrupt

Start
    BL  PLL_Init                    ; 80 MHz clock
    ; activate clock for Port F
    LDR R1, =SYSCTL_RCGCGPIO_R      ; activate Port F
    LDR R0, [R1]                   
    ORR R0, R0, #0x20              
    STR R0, [R1]                   
    NOP
    NOP                             ; allow time to finish activating
    ; set direction register
    LDR R1, =GPIO_PORTF_DIR_R      
    LDR R0, [R1]                   
    ORR R0, R0, #0x04               ; make PF2 output
    STR R0, [R1]               
    ; regular port function
    LDR R1, =GPIO_PORTF_AFSEL_R    
    LDR R0, [R1]               
    BIC R0, R0, #0x04               ; disable alt funct on PF2
    STR R0, [R1]                
    ; enable digital port
    LDR R1, =GPIO_PORTF_DEN_R    
    LDR R0, [R1]                   
    ORR R0, R0, #0x04               ; enable digital I/O on PF2
    STR R0, [R1]                
    ; configure as GPIO
    LDR R1, =GPIO_PORTF_PCTL_R    
    LDR R0, [R1]                
    BIC R0, R0, #0x00000F00         ; clear port control field for PF2
    STR R0, [R1]                   
    ; disable analog functionality
    LDR R1, =GPIO_PORTF_AMSEL_R    
    MOV R0, #0                      ; disable analog functionality on PF
    STR R0, [R1]                 
    ; enable Timer0A
    LDR R0, =80000                  ; initialize Timer0A for 1000 Hz interrupts
    BL  Timer0A_Init                ; enable Timer0A
    CPSIE I                         ; enable interrupts and configurable fault handlers (clear PRIMASK)
loop
    WFI                             ; wait for interrupt
    B   loop                        ; unconditional branch to 'loop'

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
