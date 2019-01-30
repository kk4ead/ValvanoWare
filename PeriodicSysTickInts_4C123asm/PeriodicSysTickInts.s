; PeriodicSysTickInts.s
; Runs on LM4F120/TM4C123
; Use the SysTick timer to request interrupts at a particular period.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;   Volume 1, Program 9.7
   
;   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;   Volume 2, Program 5.12, section 5.7
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

; oscilloscope or LED connected to PF2 for period measurement

        IMPORT   PLL_Init
        IMPORT   SysTick_Init

GPIO_PORTF2        EQU 0x40025010
GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

        AREA    DATA, ALIGN=4
Counts  SPACE   4                   ; records number of SysTick interrupts
        EXPORT  Counts              ; global only for observation using debugger
  ALIGN                             ; make sure the end of this section is aligned

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  SysTick_Handler
        EXPORT  Start
SysTick_Handler
    ; toggle LED
    LDR R1, =GPIO_PORTF_DATA_R     
    LDR R0, [R1]                   
    EOR R0, R0, #0x04               ; toggle PF2
    STR R0, [R1]                  
    EOR R0, R0, #0x04               ; toggle PF2
    STR R0, [R1]               
    ; increment Counts
    LDR R2, =Counts              
    LDR R3, [R2]                  
    ADD R3, R3, #1                  ; Counts = Counts + 1
    STR R3, [R2]                    ; overflows after 49 days
    EOR R0, R0, #0x04               ; toggle PF2
    STR R0, [R1]             
    BX  LR                          ; return from interrupt

Start
    BL  PLL_Init                    ; 80 MHz clock
    ; activate clock for Port F
    LDR R1, =SYSCTL_RCGCGPIO_R      
    LDR R0, [R1]                   
    ORR R0, R0, #0x20               ; set bit 5, activate Port  F
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
    ORR R0, R0, #0x04               ; nable digital I/O on PF2
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
    ; initialize Counts
    LDR R1, =Counts          
    MOV R0, #0                 
    STR R0, [R1]                    ; Counts = 0
    ; enable SysTick
    LDR R0, =80000                  ; initialize SysTick timer for 1,000 Hz interrupts
    BL  SysTick_Init                ; enable SysTick
    CPSIE I                         ; enable interrupts and configurable fault handlers (clear PRIMASK)
loop
    WFI                             ; wait for interrupt
    B   loop                        ; unconditional branch to 'loop'

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
