; Squarewaves.s
; Runs on LM4F120/TM4C123
; Initialize PF1 and PF2 as outputs with different initial values,
; then toggle them to produce two out of phase square waves.  The
; PLL was used to give the square waves more consistent timing when
; this example was done on the LM3S1968 because its internal
; oscillator can drift by +/- 30%.  However, by default the LM4F120
; gets its clock from a 16 MHz precision internal oscillator, which
; is accurate to +/- 1% to 3% across temperature.  The PLL will be
; discussed later in Chapter 4.
; Daniel Valvano
; September 12, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 4.4
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

; logic analyzer connected to PF1 and PF2

        IMPORT   PLL_Init

GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
Start
;    BL  PLL_Init                    ; set system clock to 50 MHz
     LDR R1, =SYSCTL_RCGCGPIO_R      ; 1) activate clock for Port F
     LDR R0, [R1]                    
     ORR R0, R0, #0x20               ; set bit 5 to turn on clock
     STR R0, [R1]                    
     NOP                             
     NOP                             ; allow time for clock to finish
                                     ; 2) no need to unlock PF1,PF2
     LDR R1, =GPIO_PORTF_AMSEL_R     ; 3) disable analog functionality
     LDR R0, [R1]                    
     BIC R0, #0x06                   ; 0 means analog is off
     STR R0, [R1]                    
     LDR R1, =GPIO_PORTF_PCTL_R      ; 4) configure as GPIO
     LDR R0, [R1]                    
     BIC R0, #0x00000FF0             ; 0 means configure PF1,PF2 as GPIO
     STR R0, [R1]                    
     LDR R1, =GPIO_PORTF_DIR_R       ; 5) set direction register
     LDR R0, [R1]                    
     ORR R0, #0x06                   ; PF1,PF2 output
     STR R0, [R1]                    
     LDR R1, =GPIO_PORTF_AFSEL_R     ; 6) regular port function
     LDR R0, [R1]                    
     BIC R0, #0x06                   ; 0 means disable alternate function 
     STR R0, [R1]                    
     LDR R1, =GPIO_PORTF_DEN_R       ; 7) enable Port F digital port
     LDR R0, [R1]                    
     ORR R0, #0x06                   ; 1 means enable digital I/O
     STR R0, [R1]                    

     LDR R1, =GPIO_PORTF_DATA_R                   
     MOV R0, #0x02                   ; PF2=0, PF1=1
loop STR R0, [R1]                    ; write PF1,PF2
     EOR R0, R0, #0x06               ; toggle PF1,PF2
     B   loop

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
