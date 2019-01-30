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
   .thumb
   .text
   .align 2
NVIC_EN0_INT5      .equ 0x00000020   ; Interrupt 5 enable
NVIC_EN0_R         .field 0xE000E100,32   ; IRQ 0 to 31 Set Enable Register
NVIC_PRI1_R        .field 0xE000E404,32   ; IRQ 4 to 7 Priority Register
GPIO_PORTA_AFSEL_R .field 0x40004420,32
GPIO_PORTA_DEN_R   .field 0x4000451C,32
GPIO_PORTA_AMSEL_R .field 0x40004528,32
GPIO_PORTA_PCTL_R  .field 0x4000452C,32
UART0_DR_R         .field 0x4000C000,32
UART0_FR_R         .field 0x4000C018,32
UART0_IBRD_R       .field 0x4000C024,32
UART0_FBRD_R       .field 0x4000C028,32
UART0_LCRH_R       .field 0x4000C02C,32
UART0_CTL_R        .field 0x4000C030,32
UART0_IFLS_R       .field 0x4000C034,32
UART0_IM_R         .field 0x4000C038,32
UART0_RIS_R        .field 0x4000C03C,32
UART0_ICR_R        .field 0x4000C044,32
UART_FR_RXFF       .equ 0x00000040   ; UART Receive FIFO Full
UART_FR_TXFF       .equ 0x00000020   ; UART Transmit FIFO Full
UART_FR_RXFE       .equ 0x00000010   ; UART Receive FIFO Empty
UART_LCRH_WLEN_8   .equ 0x00000060   ; 8 bit word length
UART_LCRH_FEN      .equ 0x00000010   ; UART Enable FIFOs
UART_CTL_UARTEN    .equ 0x00000001   ; UART Enable
UART_IFLS_RX1_8    .equ 0x00000000   ; RX FIFO >= 1/8 full
UART_IFLS_TX1_8    .equ 0x00000000   ; TX FIFO <= 1/8 full
UART_IM_RTIM       .equ 0x00000040   ; UART Receive Time-Out Interrupt
                                    ; Mask
UART_IM_TXIM       .equ 0x00000020   ; UART Transmit Interrupt Mask
UART_IM_RXIM       .equ 0x00000010   ; UART Receive Interrupt Mask
UART_RIS_RTRIS     .equ 0x00000040   ; UART Receive Time-Out Raw
                                    ; Interrupt Status
UART_RIS_TXRIS     .equ 0x00000020   ; UART Transmit Raw Interrupt
                                    ; Status
UART_RIS_RXRIS     .equ 0x00000010   ; UART Receive Raw Interrupt
                                    ; Status
UART_ICR_RTIC      .equ 0x00000040   ; Receive Time-Out Interrupt Clear
UART_ICR_TXIC      .equ 0x00000020   ; Transmit Interrupt Clear
UART_ICR_RXIC      .equ 0x00000010   ; Receive Interrupt Clear
SYSCTL_RCGCGPIO_R  .field 0x400FE608,32
SYSCTL_RCGCUART_R  .field 0x400FE618,32

        .global   DisableInterrupts  ; Disable interrupts
        .global   EnableInterrupts   ; Enable interrupts
        .global   StartCritical      ; previous I bit, disable interrupts
        .global   EndCritical        ; restore I bit to previous value
        .global   WaitForInterrupt   ; low power mode

; properties from FIFO.c
                           ; size of the FIFOs (must be power of 2)
FIFOSIZE    .equ 16         ; (copy this value from both places in FIFO.s)
FIFOSUCCESS .equ  1         ; return value on success
FIFOFAIL    .equ  0         ; return value on failure

; functions from FIFO.s
        .global   TxFifo_Init
        .global   TxFifo_Put
        .global   TxFifo_Get
        .global   TxFifo_Size
        .global   RxFifo_Init
        .global   RxFifo_Put
        .global   RxFifo_Get
        .global   RxFifo_Size

; standard ASCII symbols
CR                 .equ 0x0D
LF                 .equ 0x0A
BS                 .equ 0x08
ESC                .equ 0x1B
SPA                .equ 0x20
DEL                .equ 0x7F


        .global UART_Init
        .global UART_InChar
        .global UART_OutChar
        .global UART0_Handler



;------------UART_Init------------
; Initialize UART0
; Baud rate is 115200 bits/sec
; Input: none
; Output: none
; Modifies: R0, R1
; Assumes: 50 MHz system clock
UART_Init:  .asmfunc
    PUSH {LR}                       ; save current value of LR
    BL  DisableInterrupts           ; disable all interrupts (critical section)
    ; activate clock for UART0
    LDR R1, SYSCTL_RCGCUART_R       ; R1 = &SYSCTL_RCGCUART_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x01               ; enable UART0
    STR R0, [R1]                    ; [R1] = R0
    ; activate clock for port A
    LDR R1, SYSCTL_RCGCGPIO_R       ; R1 = &SYSCTL_RCGCGPIO_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x01               ; enable Port A
    STR R0, [R1]                    ; [R1] = R0
    ; initialize empty FIFOs
    BL  RxFifo_Init
    BL  TxFifo_Init
    ; disable UART
    LDR R1, UART0_CTL_R             ; R1 = &UART0_CTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #UART_CTL_UARTEN    ; R0 = R0&~UART_CTL_UARTEN (disable UART)
    STR R0, [R1]                    ; [R1] = R0
    ; set the baud rate (equations on p845 of datasheet)
    LDR R1, UART0_IBRD_R            ; R1 = &UART0_IBRD_R
    MOV R0, #27                     ; R0 = IBRD = int(50,000,000 / (16 * 115,200)) = int(27.1267)
    STR R0, [R1]                    ; [R1] = R0
    LDR R1, UART0_FBRD_R            ; R1 = &UART0_FBRD_R
    MOV R0, #8                      ; R0 = FBRD = int(0.1267 * 64 + 0.5) = 8
    STR R0, [R1]                    ; [R1] = R0
    ; configure Line Control Register settings
    LDR R1, UART0_LCRH_R            ; R1 = &UART0_LCRH_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0xFF               ; R0 = R0&~0xFF (clear all fields)
                                    ; 8 bit word length, no parity bits, one stop bit, FIFOs
    ADD R0, R0, #(UART_LCRH_WLEN_8+UART_LCRH_FEN)
    STR R0, [R1]                    ; [R1] = R0
    ; configure Interrupt FIFO Level Select Register settings
    LDR R1, UART0_IFLS_R            ; R1 = &UART0_IFLS_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x3F               ; R0 = R0&~0x3F (clear TX and RX interrupt FIFO level fields)
                                    ; configure interrupt for TX FIFO <= 1/8 full
                                    ; configure interrupt for RX FIFO >= 1/8 full
    ADD R0, R0, #(UART_IFLS_TX1_8+UART_IFLS_RX1_8)
    STR R0, [R1]                    ; [R1] = R0
    ; enable interrupts to be requested upon certain conditions
    ; TX FIFO interrupt: when TX FIFO <= 2 elements (<= 1/8 full, configured above)
    ; RX FIFO interrupt; when RX FIFO >= 2 elements (>= 1/8 full, configured above)
    ; RX time-out interrupt: receive FIFO not empty and no more data received in next 32-bit timeframe
    ;               (this causes an interrupt after each keystroke, rather than every other keystroke)
    LDR R1, UART0_IM_R              ; R1 = &UART0_IM_R
    LDR R0, [R1]                    ; R0 = [R1]
                                    ; enable TX and RX FIFO interrupts and RX time-out interrupt
    ORR R0, R0, #(UART_IM_RXIM+UART_IM_TXIM+UART_IM_RTIM)
    STR R0, [R1]                    ; [R1] = R0
    ; enable UART
    LDR R1, UART0_CTL_R             ; R1 = &UART0_CTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #UART_CTL_UARTEN    ; R0 = R0|UART_CTL_UARTEN (enable UART)
    STR R0, [R1]                    ; [R1] = R0
    ; enable alternate function
    LDR R1, GPIO_PORTA_AFSEL_R     ; R1 = &GPIO_PORTA_AFSEL_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x03               ; R0 = R0|0x03 (enable alt funct on PA1-0)
    STR R0, [R1]                    ; [R1] = R0
    ; enable digital port
    LDR R1, GPIO_PORTA_DEN_R        ; R1 = &GPIO_PORTA_DEN_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #0x03               ; R0 = R0|0x03 (enable digital I/O on PA1-0)
    STR R0, [R1]                    ; [R1] = R0
    ; configure as UART
    LDR R1, GPIO_PORTA_PCTL_R       ; R1 = &GPIO_PORTA_PCTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x000000FF         ; R0 = R0&~0x000000FF (clear port control field for PA1-0)
    ADD R0, R0, #0x00000011         ; R0 = R0+0x00000011 (configure PA1-0 as UART)
    STR R0, [R1]                    ; [R1] = R0
    ; disable analog functionality
    LDR R1, GPIO_PORTA_AMSEL_R      ; R1 = &GPIO_PORTA_AMSEL_R
    MOV R0, #0                      ; R0 = 0 (disable analog functionality on PA)
    STR R0, [R1]                    ; [R1] = R0
    ; set the priority of the UART interrupt
    LDR R1, NVIC_PRI1_R             ; R1 = &NVIC_PRI1_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x0000FF00         ; R0 = R0&~0xFFFF00FF (clear NVIC priority field for UART0 interrupt)
    ADD R0, R0, #0x00004000         ; R0 = R0+0x00004000 (UART0 = priority 2; stored in bits 13-15)
    STR R0, [R1]                    ; [R1] = R0
    ; enable interrupt 5 in NVIC
    LDR R1, NVIC_EN0_R              ; R1 = &NVIC_EN0_R
    MOV R0, #NVIC_EN0_INT5           ; R0 = NVIC_EN0_INT5 (zeros written to enable register have no effect)
    STR R0, [R1]                    ; [R1] = R0
    BL  EnableInterrupts            ; enable all interrupts (end of critical section)
    POP {PC}                        ; restore previous value of LR into PC (return)
   .endasmfunc

; private helper subroutine
; copy from hardware RX FIFO to software RX FIFO
; stop when hardware RX FIFO is empty or software RX FIFO is full
; Modifies: R0, R1
copyHardwareToSoftware:  .asmfunc
    PUSH {LR}                       ; save current value of LR
h2sloop
    ; repeat the loop while (hardware receive FIFO not empty) and (software receive FIFO not full)
    LDR R1, UART0_FR_R              ; R1 = &UART0_FR_R
    LDR R0, [R1]                    ; R0 = [R1]
    AND R0, R0, #UART_FR_RXFE       ; R0 = R0&UART_FR_RXFE
    CMP R0, #UART_FR_RXFE           ; is R0 (UART0_FR_R&UART_FR_RXFE) == UART_FR_RXFE? (is hardware receive FIFO empty?)
    BEQ h2sdone                     ; if so, skip to 'h2sdone'
    BL  RxFifo_Size
    CMP R0, #(FIFOSIZE - 1)         ; is R0 (RxFifo_Size()) == (FIFOSIZE - 1)? (is software receive FIFO full?)
    BEQ h2sdone                     ; if so, skip to 'h2sdone'
    ; read a character from the hardware FIFO
    LDR R1, UART0_DR_R              ; R1 = &UART0_DR_R
    LDR R0, [R1]                    ; R0 = [R1]
    ; store R0 (UART0_DR_R) in software receive FIFO
    BL  RxFifo_Put
    B   h2sloop                     ; unconditional branch to 'h2sloop'
h2sdone
    POP {PC}                        ; restore previous value of LR into PC (return)
   .endasmfunc

; private helper subroutine
; copy from software TX FIFO to hardware TX FIFO
; stop when software TX FIFO is empty or hardware TX FIFO is full
copySoftwareToHardware:  .asmfunc
    PUSH {LR}                       ; save current value of LR
s2hloop
    ; repeat the loop while (hardware transmit FIFO not full) and (software transmit FIFO not empty)
    LDR R1, UART0_FR_R              ; R1 = &UART0_FR_R
    LDR R0, [R1]                    ; R0 = [R1]
    AND R0, R0, #UART_FR_TXFF       ; R0 = R0&UART_FR_TXFF
    CMP R0, #UART_FR_TXFF           ; is R0 (UART0_FR_R&UART_FR_TXFF) == UART_FR_TXFF? (is hardware transmit FIFO full?)
    BEQ s2hdone                     ; if so, skip to 's2hdone'
    BL  TxFifo_Size
    CMP R0, #0                      ; is R0 (TxFifo_Size()) == 0? (is software transmit FIFO empty?)
    BEQ s2hdone                     ; if so, skip to 's2hdone'
    ; read a character from the software FIFO
    PUSH {R0}                       ; allocate local variable
    MOV R0, SP                      ; R0 = SP (R0 points to local variable)
    BL  TxFifo_Get                  ; get from software transmit FIFO into pointer R0
    POP {R0}                        ; pop data into R0
    ; store R0 (data from TxFifo_Get()) in hardware transmit FIFO
    LDR R1, UART0_DR_R              ; R1 = &UART0_DR_R
    STR R0, [R1]                    ; [R1] = R0
    B   s2hloop                     ; unconditional branch to 'h2sloop'
s2hdone
    POP {PC}                        ; restore previous value of LR into PC (return)
   .endasmfunc

;------------UART_InChar------------
; input ASCII character from UART
; spin if RxFifo is empty
; Input: none
; Output: R0  character in from UART
; Very Important: The UART0 interrupt handler automatically
;  empties the hardware receive FIFO into the software FIFO as
;  the hardware gets data.  If the UART0 interrupt is
;  disabled, the software receive FIFO may become empty, and
;  this function will stall forever.
;  Ensure that the UART0 module is initialized and its
;  interrupt is enabled before calling this function.  Do not
;  use UART I/O functions within a critical section of your
;  main program.
UART_InChar:  .asmfunc
    MOV R0, #0                      ; initialize local variable
    PUSH {R0, LR}                   ; save current value of LR and allocate local variable
inCharLoop
    MOV R0, SP                      ; R0 = SP (R0 points to local variable)
    BL  RxFifo_Get                  ; get from software receive FIFO into pointer R0
    CMP R0, #FIFOFAIL               ; is R0 (RxFifo_Get()) == FIFOFAIL (value returned when FIFO empty)?
    BEQ inCharLoop                  ; if so, skip to 'inCharLoop' (spin until receive a character)
    POP {R0, PC}                    ; pop data into R0 and restore LR into PC (return)
   .endasmfunc

;------------UART_OutChar------------
; output ASCII character to UART
; spin if TxFifo is full
; Input: R0  character out to UART
; Output: none
; Modifies: R0, R1
; Very Important: The UART0 interrupt handler automatically
;  empties the software transmit FIFO into the hardware FIFO as
;  the hardware sends data.  If the UART0 interrupt is
;  disabled, the software transmit FIFO may become full, and
;  this function will stall forever.
;  Ensure that the UART0 module is initialized and its
;  interrupt is enabled before calling this function.  Do not
;  use UART I/O functions within a critical section of your
;  main program.
UART_OutChar:  .asmfunc
    PUSH {R4, LR}                   ; save current value of R4 and LR
    MOV R4, R0                      ; R4 = R0 (save the output character)
outCharLoop
    MOV R0, R4                      ; R0 = R4 (recall the output character)
    BL  TxFifo_Put                  ; store R0 (output character) in software transmit FIFO
    CMP R0, #FIFOFAIL               ; is R0 (TxFifo_Put()) == FIFOFAIL (value returned when FIFO full)?
    BEQ outCharLoop                 ; if so, skip to 'outCharLoop' (spin until space in software transmit FIFO)
    LDR R4, UART0_IM_R              ; R4 = &UART0_IM_R
    LDR R0, [R4]                    ; R0 = [R4]
    BIC R0, R0, #UART_IM_TXIM       ; R0 = R0&~UART_IM_TXIM (disable TX FIFO interrupt)
    STR R0, [R4]                    ; [R4] = R0
    BL  copySoftwareToHardware      ; private helper subroutine
    LDR R0, [R4]                    ; R0 = [R4]
    ORR R0, R0, #UART_IM_TXIM       ; R0 = R0|UART_IM_TXIM (enable TX FIFO interrupt)
    STR R0, [R4]                    ; [R4] = R0
    POP {R4, PC}                    ; restore previous value of R4 into R4 and LR into PC (return)
   .endasmfunc

;------------UART0_Handler------------
; at least one of three things has happened:
; hardware TX FIFO goes from 3 to 2 or less items
; hardware RX FIFO goes from 1 to 2 or more items
; UART receiver has timed out
UART0_Handler:  .asmfunc
    PUSH {LR}                       ; save current value of LR
    ; check the flags to determine which interrupt condition occurred
handlerCheck0
    LDR R1, UART0_RIS_R             ; R1 = &UART0_RIS_R
    LDR R0, [R1]                    ; R0 = [R1]
    AND R0, R0, #UART_RIS_TXRIS     ; R0 = R0&UART_RIS_TXRIS
    CMP R0, #UART_RIS_TXRIS         ; is R0 (UART0_RIS_R&UART_RIS_TXRIS) == UART_RIS_TXRIS? (does hardware TX FIFO have <= 2 items?)
    BNE handlerCheck1               ; if not, skip to 'handlerCheck1' and check the next flag
    ; acknowledge TX FIFO interrupt
    LDR R1, UART0_ICR_R             ; R1 = &UART0_ICR_R
    MOV R0, #UART_ICR_TXIC          ; R0 = UART_ICR_TXIC (zeros written to interrupt clear register have no effect)
    STR R0, [R1]                    ; [R1] = R0
    ; copy from software TX FIFO to hardware TX FIFO
    BL  copySoftwareToHardware      ; private helper subroutine
    ; if the software transmit FIFO is now empty, disable TX FIFO interrupt
    ; UART_OutChar() will re-enable the TX FIFO interrupt when it is needed
    BL  TxFifo_Size
    CMP R0, #0                      ; is R0 (TxFifo_Size()) == 0? (is software transmit FIFO empty?)
    BNE handlerCheck1               ; if not, skip to 'handlerCheck1'
    LDR R1, UART0_IM_R              ; R1 = &UART0_IM_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #UART_IM_TXIM       ; R0 = R0&~UART_IM_TXIM (disable TX FIFO interrupt)
    STR R0, [R1]                    ; [R1] = R0
handlerCheck1
    LDR R1, UART0_RIS_R             ; R1 = &UART0_RIS_R
    LDR R0, [R1]                    ; R0 = [R1]
    AND R0, R0, #UART_RIS_RXRIS     ; R0 = R0&UART_RIS_RXRIS
    CMP R0, #UART_RIS_RXRIS         ; is R0 (UART0_RIS_R&UART_RIS_RXRIS) == UART_RIS_RXRIS? (does hardware RX FIFO have >= 2 items?)
    BNE handlerCheck2               ; if not, skip to 'handlerCheck2' and check the next flag
    ; acknowledge RX FIFO interrupt
    LDR R1, UART0_ICR_R             ; R1 = &UART0_ICR_R
    MOV R0, #UART_ICR_RXIC          ; R0 = UART_ICR_RXIC (zeros written to interrupt clear register have no effect)
    STR R0, [R1]                    ; [R1] = R0
    ; copy from hardware RX FIFO to software RX FIFO
    BL  copyHardwareToSoftware      ; private helper subroutine
handlerCheck2
    LDR R1, UART0_RIS_R             ; R1 = &UART0_RIS_R
    LDR R0, [R1]                    ; R0 = [R1]
    AND R0, R0, #UART_RIS_RTRIS     ; R0 = R0&UART_RIS_RTRIS
    CMP R0, #UART_RIS_RTRIS         ; is R0 (UART0_RIS_R&UART_RIS_RTRIS) == UART_RIS_RTRIS? (did the receiver timeout?)
    BNE handlerDone                 ; if not, skip to 'handlerDone'
    ; acknowledge receiver timeout interrupt
    LDR R1, UART0_ICR_R             ; R1 = &UART0_ICR_R
    MOV R0, #UART_ICR_RTIC           ; R0 = UART_ICR_RTIC (zeros written to interrupt clear register have no effect)
    STR R0, [R1]                    ; [R1] = R0
    ; copy from hardware RX FIFO to software RX FIFO
    BL  copyHardwareToSoftware      ; private helper subroutine
handlerDone
    POP {PC}                        ; restore previous value of LR into PC (return from interrupt)
   .endasmfunc


    .end                             ; end of file
