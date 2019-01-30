; UARTIntTestMain.s
; Runs on LM4F120/TM4C123
; Tests the UART0 to implement bidirectional data transfer to and from a
; computer running PuTTY.  This time, interrupts and FIFOs
; are used.
; This file is named "UART2" because it is the second UART example.
; It is not related to the UART2 module on the microcontroller.
; Daniel Valvano
; September 12, 2013

;  This example accompanies the book
;  "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
;  ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;  Program 5.11 Section 5.6, Program 3.10
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

; U0Rx (VCP receive) connected to PA0
; U0Tx (VCP transmit) connected to PA1

; standard ASCII symbols
CR                 EQU 0x0D
LF                 EQU 0x0A
BS                 EQU 0x08
ESC                EQU 0x1B
SPA                EQU 0x20
DEL                EQU 0x7F

; functions in PLL.s
        IMPORT PLL_Init

; functions UART.s
        IMPORT UART_Init
        IMPORT UART_InChar
        IMPORT UART_OutChar

        AREA    DATA, ALIGN=2

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT Start

    ALIGN                           ; make sure the end of this section is aligned

;---------------------OutCRLF---------------------
; Output a CR,LF to UART to go to a new line
; Input: none
; Output: none
OutCRLF
    PUSH {LR}                       ; save current value of LR
    MOV R0, #CR                     ; R0 = CR (<carriage return>)
    BL  UART_OutChar                ; send <carriage return> to the UART
    MOV R0, #LF                     ; R0 = LF (<line feed>)
    BL  UART_OutChar                ; send <line feed> to the UART
    POP {PC}                        ; restore previous value of LR into PC (return)

Start
    BL  PLL_Init                    ; set system clock to 50 MHz
    BL  UART_Init                   ; initialize UART
    BL  OutCRLF                     ; go to a new line
    ; print the uppercase alphabet
    MOV R4, #'A'                    ; R4 = 'A'
uppercaseLoop
    MOV R0, R4                      ; R0 = R4
    BL  UART_OutChar                ; send the character (R4) to the UART
    ADD R4, R4, #1                  ; R4 = R4 + 1 (go to the next character in the alphabet)
    CMP R4, #'Z'                    ; is R4 (character) <= 'Z'?
    BLS uppercaseLoop               ; if so, skip to 'uppercaseLoop'
    BL  OutCRLF                     ; go to a new line
    MOV R0, #'>'                    ; R0 = '>'
    BL  UART_OutChar                ; send the character ('>') to the UART
loop
    ; echo each input

    BL  UART_InChar                 ; input a characture
    BL  UART_OutChar                ; echo the character back to the UART

    B   loop                        

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
