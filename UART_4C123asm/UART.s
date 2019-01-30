; UART.s
; Runs on LM4F120/TM4C123
; Use UART0 to implement bidirectional data transfer to and from a
; computer running HyperTerminal.  This time, interrupts and FIFOs
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

NVIC_EN0_INT5      EQU 0x00000020   ; Interrupt 5 enable
NVIC_EN0_R         EQU 0xE000E100   ; IRQ 0 to 31 Set Enable Register
NVIC_PRI1_R        EQU 0xE000E404   ; IRQ 4 to 7 Priority Register
GPIO_PORTA_AFSEL_R EQU 0x40004420
GPIO_PORTA_DEN_R   EQU 0x4000451C
GPIO_PORTA_AMSEL_R EQU 0x40004528
GPIO_PORTA_PCTL_R  EQU 0x4000452C
UART0_DR_R         EQU 0x4000C000
UART0_FR_R         EQU 0x4000C018
UART0_IBRD_R       EQU 0x4000C024
UART0_FBRD_R       EQU 0x4000C028
UART0_LCRH_R       EQU 0x4000C02C
UART0_CTL_R        EQU 0x4000C030
UART0_IFLS_R       EQU 0x4000C034
UART0_IM_R         EQU 0x4000C038
UART0_RIS_R        EQU 0x4000C03C
UART0_ICR_R        EQU 0x4000C044
UART_FR_RXFF       EQU 0x00000040   ; UART Receive FIFO Full
UART_FR_TXFF       EQU 0x00000020   ; UART Transmit FIFO Full
UART_FR_RXFE       EQU 0x00000010   ; UART Receive FIFO Empty
UART_LCRH_WLEN_8   EQU 0x00000060   ; 8 bit word length
UART_LCRH_FEN      EQU 0x00000010   ; UART Enable FIFOs
UART_CTL_UARTEN    EQU 0x00000001   ; UART Enable
UART_IFLS_RX1_8    EQU 0x00000000   ; RX FIFO >= 1/8 full
UART_IFLS_TX1_8    EQU 0x00000000   ; TX FIFO <= 1/8 full
UART_IM_RTIM       EQU 0x00000040   ; UART Receive Time-Out Interrupt
                                    ; Mask
UART_IM_TXIM       EQU 0x00000020   ; UART Transmit Interrupt Mask
UART_IM_RXIM       EQU 0x00000010   ; UART Receive Interrupt Mask
UART_RIS_RTRIS     EQU 0x00000040   ; UART Receive Time-Out Raw
                                    ; Interrupt Status
UART_RIS_TXRIS     EQU 0x00000020   ; UART Transmit Raw Interrupt
                                    ; Status
UART_RIS_RXRIS     EQU 0x00000010   ; UART Receive Raw Interrupt
                                    ; Status
UART_ICR_RTIC      EQU 0x00000040   ; Receive Time-Out Interrupt Clear
UART_ICR_TXIC      EQU 0x00000020   ; Transmit Interrupt Clear
UART_ICR_RXIC      EQU 0x00000010   ; Receive Interrupt Clear
SYSCTL_RCGCGPIO_R  EQU 0x400FE608
SYSCTL_RCGCUART_R  EQU 0x400FE618

        IMPORT   DisableInterrupts  ; Disable interrupts
        IMPORT   EnableInterrupts   ; Enable interrupts
        IMPORT   StartCritical      ; previous I bit, disable interrupts
        IMPORT   EndCritical        ; restore I bit to previous value
        IMPORT   WaitForInterrupt   ; low power mode


; standard ASCII symbols
CR                 EQU 0x0D
LF                 EQU 0x0A
BS                 EQU 0x08
ESC                EQU 0x1B
SPA                EQU 0x20
DEL                EQU 0x7F

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT UART_Init
        EXPORT UART_InChar
        EXPORT UART_OutChar

; require C function calls to preserve the 8-byte alignment of 8-byte data objects
        PRESERVE8

;------------UART_Init------------
; Initialize UART0
; Baud rate is 115200 bits/sec
; Input: none
; Output: none
; Modifies: R0, R1
; Assumes: 50 MHz system clock
UART_Init
    PUSH {LR}                       ; save current value of LR
    ; activate clock for UART0
    LDR R1, =SYSCTL_RCGCUART_R      ; R1 = &SYSCTL_RCGCUART_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x01               ; enable UART0
    STR R0, [R1]                    ; [R1] = R0
    ; activate clock for port A
    LDR R1, =SYSCTL_RCGCGPIO_R      ; R1 = &SYSCTL_RCGCGPIO_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x01               ; enable Port A
    STR R0, [R1]                    ; [R1] = R0

    ; disable UART
    LDR R1, =UART0_CTL_R            ; R1 = &UART0_CTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #UART_CTL_UARTEN    ; R0 = R0&~UART_CTL_UARTEN (disable UART)
    STR R0, [R1]                    ; [R1] = R0
    ; set the baud rate (equations on p845 of datasheet)
    LDR R1, =UART0_IBRD_R           ; R1 = &UART0_IBRD_R
    MOV R0, #27                     ; R0 = IBRD = int(50,000,000 / (16 * 115,200)) = int(27.1267)
    STR R0, [R1]                    ; [R1] = R0
    LDR R1, =UART0_FBRD_R           ; R1 = &UART0_FBRD_R
    MOV R0, #8                      ; R0 = FBRD = int(0.1267 * 64 + 0.5) = 8
    STR R0, [R1]                    ; [R1] = R0
    ; configure Line Control Register settings
    LDR R1, =UART0_LCRH_R           ; R1 = &UART0_LCRH_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0xFF               ; R0 = R0&~0xFF (clear all fields)
                                    ; 8 bit word length, no parity bits, one stop bit, FIFOs
    ADD R0, R0, #(UART_LCRH_WLEN_8+UART_LCRH_FEN)
    STR R0, [R1]                    ; [R1] = R0
    ; enable UART
    LDR R1, =UART0_CTL_R            ; R1 = &UART0_CTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #UART_CTL_UARTEN    ; R0 = R0|UART_CTL_UARTEN (enable UART)
    STR R0, [R1]                    ; [R1] = R0
    ; enable alternate function
    LDR R1, =GPIO_PORTA_AFSEL_R     ; R1 = &GPIO_PORTA_AFSEL_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x03               ; R0 = R0|0x03 (enable alt funct on PA1-0)
    STR R0, [R1]                    ; [R1] = R0
    ; enable digital port
    LDR R1, =GPIO_PORTA_DEN_R       ; R1 = &GPIO_PORTA_DEN_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x03               ; R0 = R0|0x03 (enable digital I/O on PA1-0)
    STR R0, [R1]                    ; [R1] = R0
    ; configure as UART
    LDR R1, =GPIO_PORTA_PCTL_R      ; R1 = &GPIO_PORTA_PCTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x000000FF         ; R0 = R0&~0x000000FF (clear port control field for PA1-0)
    ADD R0, R0, #0x00000011         ; R0 = R0+0x00000011 (configure PA1-0 as UART)
    STR R0, [R1]                    ; [R1] = R0
    ; disable analog functionality
    LDR R1, =GPIO_PORTA_AMSEL_R     ; R1 = &GPIO_PORTA_AMSEL_R
    MOV R0, #0                      ; R0 = 0 (disable analog functionality on PA)
    STR R0, [R1]                    ; [R1] = R0

    POP {PC}                        ; restore previous value of LR into PC (return)


;------------UART_InChar------------
; input ASCII character from UART
; spin if no data available i
; Input: none
; Output: R0  character in from UART
UART_InChar
       LDR  R1,=UART0_FR_R
InLoop LDR  R2,[R1]    ; read FR
       ANDS R2,#0x0010
       BNE  InLoop     ; wait until RXFE is 0
       LDR  R1,=UART0_DR_R
       LDR  R0,[R1]    ; read DR
       BX   LR


;------------UART_OutChar------------
; output ASCII character to UART
; spin if UART transmit FIFO is full
; Input: R0  character out to UART
; Output: none
; Modifies: R0, R1

UART_OutChar
       LDR  R1,=UART0_FR_R
OLoop  LDR  R2,[R1]    ; read FR
       ANDS R2,#0x0020
       BNE  OLoop      ; wait until TXFF is 0
       LDR  R1,=UART0_DR_R
       STR  R0,[R1]    ; write DR
       BX   LR


    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
