; main.s
; Runs on LM4F120/TM4C123
; Provide functions that initialize a GPIO pin and turn it on and off.
; Use bit-banded I/O.
; Daniel Valvano
; September 12, 2013

; This example accompanies the book
;   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;   Volume 1 Program 4.3, Figure 4.14

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

; solid state relay connected to PF2

GPIO_PORTF_DATA_R  EQU 0x400253FC
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_PUR_R   EQU 0x40025510
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
PF2                EQU 0x40025010
PF4                EQU 0x40025040
SYSCTL_RCGCGPIO_R  EQU 0x400FE608

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
; This is an extremely simple test program to demonstrate that the SSR
; can turn on and off.  Button de-bouncing is not addressed.
; built-in negative logic switch connected to PF4
Start
    BL  SSR_Init                    ; initialize PF2 and make it output
    ; set direction register
    LDR R1, =GPIO_PORTF_DIR_R       ; R1 = &GPIO_PORTF_DIR_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x10               ; R0 = R0&~0x10 (make PF4 input; PF4 built-in button #1)
    STR R0, [R1]                    ; [R1] = R0
    ; regular port function
    LDR R1, =GPIO_PORTF_AFSEL_R     ; R1 = &GPIO_PORTF_AFSEL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x10               ; R0 = R0&~0x10 (disable alt funct on PF4)
    STR R0, [R1]                    ; [R1] = R0
    ; pull-up resistors on switch pins
    LDR R1, =GPIO_PORTF_PUR_R       ; R1 = &GPIO_PORTF_PUR_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x10               ; R0 = R0|0x10 (enable pull-up on PF4)
    STR R0, [R1]                    ; [R1] = R0
    ; enable digital port
    LDR R1, =GPIO_PORTF_DEN_R       ; R1 = &GPIO_PORTF_DEN_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x10               ; R0 = R0|0x10 (enable digital I/O on PF4)
    STR R0, [R1]                    ; [R1] = R0
    ; configure as GPIO
    LDR R1, =GPIO_PORTF_PCTL_R      ; R1 = &GPIO_PORTF_PCTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x000F0000         ; R0 = R0&~0x000F0000 (clear port control field for PF4)
    ADD R0, R0, #0x00000000         ; R0 = R0+0x00000000 (configure PF4 as GPIO)
    STR R0, [R1]                    ; [R1] = R0
    LDR R4, =PF4                    ; R4 = &PF4
loop                                ; in this loop, the appliance (PF2) toggles when the switch is released
    BL  SSR_On
waitforpress1                       ; proceed only when the button is pressed
    LDR R0, [R4]                    ; R0 = [R4] (read status of PF4)
    CMP R0, #0x10                   ; R0 == 0x10?
    BEQ waitforpress1               ; if so, spin
waitforrelease1                     ; proceed only when the button is released
    LDR R0, [R4]                    ; R0 = [R4] (read status of PF4)
    CMP R0, #0x10                   ; R0 != 0x10?
    BNE waitforrelease1             ; if so, spin
    BL  SSR_Off
waitforpress2                       ; proceed only when the button is pressed
    LDR R0, [R4]                    ; R0 = [R4] (read status of PF4)
    CMP R0, #0x10                   ; R0 == 0x10?
    BEQ waitforpress2               ; if so, spin
waitforrelease2                     ; proceed only when the button is released
    LDR R0, [R4]                    ; R0 = [R4] (read status of PF4)
    CMP R0, #0x10                   ; R0 != 0x10?
    BNE waitforrelease2             ; if so, spin
    B   loop

;------------SSR_Init------------
; Make PF2 an output, enable digital I/O, ensure alt. functions off.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_Init
    LDR R1, =SYSCTL_RCGCGPIO_R      ; 1) activate clock for Port F
    LDR R0, [R1]                 
    ORR R0, R0, #0x20               ; set bit 5 to turn on clock
    STR R0, [R1]                  
    NOP
    NOP
    NOP                             ; allow time for clock to finish
                                    ; 2) no need to unlock PF2
    LDR R1, =GPIO_PORTF_AMSEL_R     ; 3) disable analog functionality
    LDR R0, [R1]                    
    BIC R0, #0x04                   ; 0 means analog is off
    STR R0, [R1]                    
    LDR R1, =GPIO_PORTF_PCTL_R      ; 4) configure as GPIO
    LDR R0, [R1]                    
    BIC R0, #0x00000F00             ;  0 means configure Port F as GPIO
    STR R0, [R1]                  
    LDR R1, =GPIO_PORTF_DIR_R       ; 5) set direction register
    LDR R0, [R1]                    
    ORR R0,#0x04                    ; PF2 output
    STR R0, [R1]                    
    LDR R1, =GPIO_PORTF_AFSEL_R     ; 6) regular port function
    LDR R0, [R1]                    
    BIC R0, #0x04                   ; 0 means disable alternate function 
    STR R0, [R1]                    
    LDR R1, =GPIO_PORTF_DEN_R       ; 7) enable Port F digital port
    LDR R0, [R1]                    
    ORR R0,#0x04                    ; 1 means enable digital I/O
    STR R0, [R1]    
    BX  LR
;------------SSR_On------------
; Make PF2 high.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_On
    LDR R1, =PF2                    ; R1 = &PF2
    MOV R0, #0x04                   ; R0 = 0x04 (turn on the appliance)
    STR R0, [R1]                    ; [R1] = R0, write to PF2
    BX  LR                          ; return

;------------SSR_Off------------
; Make PF2 low.
; Input: none
; Output: none
; Modifies: R0, R1
SSR_Off
    LDR R1, =PF2                    ; R1 = &PF2
    MOV R0, #0x00                   ; R0 = 0x00 (turn off the appliance)
    STR R0, [R1]                    ; [R1] = R0, write to PF2
    BX  LR                          ; return
    
SSR_Toggle
    LDR R1, =PF2                    ; R1 is 0x40025010
    LDR R0, [R1]                    ; previous value
    EOR R0, R0, #0x04               ; flip bit 2
    STR R0, [R1]                    ; affect just PF2
    BX  LR

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
