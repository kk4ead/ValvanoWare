; SwitchTestMain.s
; Runs on LM4F120/TM4C123
; Test the switch initialization functions by setting the LED
; color according to the status of the switches.
; Daniel and Jonathan Valvano
; September 12, 2013

; This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Section 4.2.2, Program 4.2, Figure 4.9

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

; negative logic switches connected to PF0 and PF4 on the Launchpad
; red LED connected to PF1 on the Launchpad
; blue LED connected to PF2 on the Launchpad
; green LED connected to PF3 on the Launchpad
; NOTE: The NMI (non-maskable interrupt) is on PF0.  That means that
; the Alternate Function Select, Pull-Up Resistor, Pull-Down Resistor,
; and Digital Enable are all locked for PF0 until a value of 0x4C4F434B
; is written to the Port F GPIO Lock Register.  After Port F is
; unlocked, bit 0 of the Port F GPIO Commit Register must be set to
; allow access to PF0's control registers.  On the LM4F120, the other
; bits of the Port F GPIO Commit Register are hard-wired to 1, meaning
; that the rest of Port F can always be freely re-configured at any
; time.  Requiring this procedure makes it unlikely to accidentally
; re-configure the JTAG and NMI pins as GPIO, which can lock the
; debugger out of the processor and make it permanently unable to be
; debugged or re-programmed.

        IMPORT  Board_Init
        IMPORT  Board_Input
        IMPORT  Switch_Init
        IMPORT  Switch_Input
GPIO_PORTF_DIR_R   EQU 0x40025400
GPIO_PORTF_AFSEL_R EQU 0x40025420
GPIO_PORTF_DEN_R   EQU 0x4002551C
GPIO_PORTF_AMSEL_R EQU 0x40025528
GPIO_PORTF_PCTL_R  EQU 0x4002552C
PF0       EQU 0x40025004
PF1       EQU 0x40025008
PF2       EQU 0x40025010
PF3       EQU 0x40025020
PF4       EQU 0x40025040
LEDS      EQU 0x40025038
RED       EQU 0x02
BLUE      EQU 0x04
GREEN     EQU 0x08
SWITCHES  EQU 0x40025044
SW1       EQU 0x10                 ; on the left side of the Launchpad board
SW2       EQU 0x01                 ; on the right side of the Launchpad board

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
Start 
    BL  Switch_Init   ; initialize PA5 is input
    BL  Switch_Input  ; R0 returned as 0x20 or 0x00
    BL  Switch_Input  ; R0 returned as 0x20 or 0x00

;----------------------------------------------------------------
    BL  Board_Init                  ; initialize PF0 and PF4 and make them inputs
    ; set direction register
    LDR R1, =GPIO_PORTF_DIR_R       ; R1 = &GPIO_PORTF_DIR_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #(RED+BLUE+GREEN)   ; R0 = R0|(RED|BLUE|GREEN) (make PF3-1 output; PF3-1 built-in LEDs)
    STR R0, [R1]                    ; [R1] = R0
    ; regular port function
    LDR R1, =GPIO_PORTF_AFSEL_R     ; R1 = &GPIO_PORTF_AFSEL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #(RED+BLUE+GREEN)   ; R0 = R0&~(RED|BLUE|GREEN) (disable alt funct on PF3-1)
    STR R0, [R1]                    ; [R1] = R0
    ; enable digital port
    LDR R1, =GPIO_PORTF_DEN_R       ; R1 = &GPIO_PORTF_DEN_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #(RED+BLUE+GREEN)   ; R0 = R0|(RED|BLUE|GREEN) (enable digital I/O on PF3-1)
    STR R0, [R1]                    ; [R1] = R0
    ; configure as GPIO
    LDR R1, =GPIO_PORTF_PCTL_R      ; R1 = &GPIO_PORTF_PCTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x0000FF00         ; R0 = R0&~0x0000FF00 (clear port control field for PF3-2)
    BIC R0, R0, #0x000000F0         ; R0 = R0&~0x000000F0 (clear port control field for PF1)
    STR R0, [R1]                    ; [R1] = R0
    ; disable analog functionality
    LDR R1, =GPIO_PORTF_AMSEL_R     ; R1 = &GPIO_PORTF_AMSEL_R
    MOV R0, #0                      ; R0 = 0 (disable analog functionality on PF)
    STR R0, [R1]                    ; [R1] = R0
    LDR R4, =LEDS                   ; R4 = &LEDS
    MOV R5, #RED                    ; R5 = RED (red LED on)
    MOV R6, #BLUE                   ; R6 = BLUE (blue LED on)
    MOV R7, #GREEN                  ; R7 = GREEN (green LED on)
    MOV R8, #0                      ; R8 = 0 (no LEDs on)
    MOV R9, #(RED+GREEN+BLUE)       ; R9 = (RED|GREEN|BLUE) (all LEDs on)
loop
    BL  Board_Input
    CMP R0, #0x01                   ; R0 == 0x01?
    BEQ sw1pressed                  ; if so, switch 1 pressed
    CMP R0, #0x10                   ; R0 == 0x10?
    BEQ sw2pressed                  ; if so, switch 2 pressed
    CMP R0, #0x00                   ; R0 == 0x00?
    BEQ bothpressed                 ; if so, both switches pressed
    CMP R0, #0x11                   ; R0 == 0x11?
    BEQ nopressed                   ; if so, neither switch pressed
                                    ; if none of the above, unexpected return value
    STR R9, [R4]                    ; [R4] = R9 = (RED|GREEN|BLUE) (all LEDs on)
    B   loop
sw1pressed
    STR R6, [R4]                    ; [R4] = R6 = BLUE (blue LED on)
    B   loop
sw2pressed
    STR R5, [R4]                    ; [R4] = R5 = RED (red LED on)
    B   loop
bothpressed
    STR R7, [R4]                    ; [R4] = R7 = GREEN (green LED on)
    B   loop
nopressed
    STR R8, [R4]                    ; [R4] = R8 = 0 (no LEDs on)
    B   loop

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
