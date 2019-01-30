; GPIO.s
; Runs on LM4F120/TM4C123
; Initialize four GPIO pins as outputs.  Continually generate output to
; drive simulated stepper motor.
; Daniel Valvano
; May 3, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Volume 1 Program 4.5

;"Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;   Volume 2 Example 2.2, Program 2.8, Figure 2.32
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

; PD3 is an output to LED3, negative logic
; PD2 is an output to LED2, negative logic
; PD1 is an output to LED1, negative logic
; PD0 is an output to LED0, negative logic

LEDS               EQU 0x4000703C   ; access PD3-PD0
GPIO_PORTD_DATA_R  EQU 0x400073FC
GPIO_PORTD_DIR_R   EQU 0x40007400
GPIO_PORTD_AFSEL_R EQU 0x40007420
GPIO_PORTD_DR8R_R  EQU 0x40007508
GPIO_PORTD_DEN_R   EQU 0x4000751C
GPIO_PORTD_AMSEL_R EQU 0x40007528
GPIO_PORTD_PCTL_R  EQU 0x4000752C
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
SYSCTL_RCGC2_GPIOD EQU 0x00000008   ; port D Clock Gating Control

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
GPIO_Init
    ; 1) activate clock for Port D
    LDR R1, =SYSCTL_RCGCGPIO_R      
    LDR R0, [R1]                   
    ORR R0, R0, #SYSCTL_RCGC2_GPIOD ; clock
    STR R0, [R1]                  
    NOP
    NOP                             ; allow time to finish activating
    ; 2) no need to unlock PD3-0
    ; 3) disable analog functionality
    LDR R1, =GPIO_PORTD_AMSEL_R    
    LDR R0, [R1]                   
    BIC R0, R0, #0x0F               ; disable analog functionality on PD3-0
    STR R0, [R1]                 
    ; 4) configure as GPIO
    LDR R1, =GPIO_PORTD_PCTL_R   
    LDR R0, [R1]                 
    MOV R2, #0x0000FFFF          
    BIC R0, R0, R2                  ; clear port control field for PD3-0
    STR R0, [R1]               

    ; 5) set direction register
    LDR R1, =GPIO_PORTD_DIR_R     
    LDR R0, [R1]                 
    ORR R0, R0, #0x0F               ; make PD3-0 output
    STR R0, [R1]              
    ; 6) regular port function
    LDR R1, =GPIO_PORTD_AFSEL_R  
    LDR R0, [R1]              
    BIC R0, R0, #0x0F               ; disable alt funct on PD3-0
    STR R0, [R1]              
    ; enable 8mA drive (only necessary for bright LEDs)
    LDR R1, =GPIO_PORTD_DR8R_R     
    LDR R0, [R1]                  
    ORR R0, R0, #0x0F               ; enable 8mA drive on PD3-0
    STR R0, [R1]                  
    ; 7) enable digital port
    LDR R1, =GPIO_PORTD_DEN_R     
    LDR R0, [R1]                
    ORR R0, R0, #0x0F               ; enable digital I/O on PD3-0
    STR R0, [R1]             
    BX  LR
    
Start
    BL  GPIO_Init
    LDR R0, =LEDS                   ; R0 = LEDS
    MOV R1, #10                     ; R1 = 10
    MOV R2, #9                      ; R2 = 9
    MOV R3, #5                      ; R3 = 5
    MOV R4, #6                      ; R4 = 6
loop
    STR R1, [R0]                    ; [R0] = R1 = 10
    STR R2, [R0]                    ; [R0] = R2 = 9
    STR R3, [R0]                    ; [R0] = R3 = 5
    STR R4, [R0]                    ; [R0] = R4 = 6
    B loop                          ; unconditional branch to 'loop'


    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
    