; LLFifoTestMain.asm
; Runs on any computer
; Test linked list FIFO
; Jonathan Valvano
; April 23, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Section 6.6.1 Programs 6.10, 6.11, 6.12, 6.13
;  Section 6.6.2 Programs 6.14, 6.15, 6.16, 6.17
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
       .thumb
       .data
       .align  2

num1   .space 4
num2   .space 4
num3   .space 4
num4   .space 4
num5   .space 4
;export for use with debugger

       .text
       .align 2
       .global  Fifo_Init
       .global  Fifo_Put
       .global  Fifo_Get
       .global  main

; subroutine called when an error is encountered
; Input: none
; Output: none
; Modifies: R4, R5
crash:   .asmfunc
    MOV R4, R14                     ; R4 = R14(LR)
    SUB R4, R4, #1                  ; R4 = R4 - 1 (R4 is memory location of the instruction following the most recent 'BL  crash' instruction)
    ADD R5, R5, #1                  ; R5 = R5 + 1 (R5 is number of crashes so far in this pass through loop)
    BX  LR                          ; return
    .endasmfunc
main:   .asmfunc
    BL  Fifo_Init                   ; initialize linked list FIFO
loop
    MOV R4, #0                      ; R4 = 0 (R4 is memory location of most recent crash)
    MOV R5, #0                      ; R5 = 0 (R5 is number of crashes so far in this pass through loop)
;*************** first tests ***************
    MOV R0, #1                      ; R0 = 1
                                    ;        First          Last  Description
    BL  Fifo_Put                    ; FIFO:  [1] [ ] [ ] [ ] [ ]  1 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip1                       ; if not, skip to 'skip1'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip1
    MOV R0, #2                      ; R0 = 2
    BL  Fifo_Put                    ; FIFO:  [1] [2] [ ] [ ] [ ]  2 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip2                       ; if not, skip to 'skip2'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip2
    LDR R0, num1Addr                ; R0 = &num1 (pointer)
    BL  Fifo_Get                    ; FIFO:  [2] [ ] [ ] [ ] [ ]  1 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip3                       ; if not, skip to 'skip3'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip3
    LDR R0, num1Addr               ; R0 = &num1 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num1 (value)
    CMP R0, #1                      ; is R0 (num1) == 1? (was 1 correctly removed from the FIFO?)
    BEQ skip4                       ; if so, skip to 'skip4'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip4
    MOV R0, #3                      ; R0 = 3
    BL  Fifo_Put                    ; FIFO:  [2] [3] [ ] [ ] [ ]  3 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip5                       ; if not, skip to 'skip5'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip5
    LDR R0, num2Addr                ; R0 = &num2 (pointer)
    BL  Fifo_Get                    ; FIFO:  [3] [ ] [ ] [ ] [ ]  2 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip6                       ; if not, skip to 'skip6'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip6
    LDR R0, num2Addr                 ; R0 = &num2 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num2 (value)
    CMP R0, #2                      ; is R0 (num2) == 2? (was 2 correctly removed from the FIFO?)
    BEQ skip7                       ; if so, skip to 'skip7'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip7
    MOV R0, #4                      ; R0 = 4
    BL  Fifo_Put                    ; FIFO:  [3] [4] [ ] [ ] [ ]  4 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip8                       ; if not, skip to 'skip8'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip8
    LDR R0, num3Addr                ; R0 = &num3 (pointer)
    BL  Fifo_Get                    ; FIFO:  [4] [ ] [ ] [ ] [ ]  3 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip9                       ; if not, skip to 'skip9'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip9
    LDR R0, num3Addr                ; R0 = &num3 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num3 (value)
    CMP R0, #3                      ; is R0 (num3) == 3? (was 3 correctly removed from the FIFO?)
    BEQ skip10                      ; if so, skip to 'skip10'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip10
    LDR R0, num4Addr                ; R0 = &num4 (pointer)
    BL  Fifo_Get                    ; FIFO:  [ ] [ ] [ ] [ ] [ ]  4 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip11                      ; if not, skip to 'skip11'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip11
    LDR R0, num4Addr                ; R0 = &num4 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num4 (value)
    CMP R0, #4                      ; is R0 (num4) == 4? (was 4 correctly removed from the FIFO?)
    BEQ skip12                      ; if so, skip to 'skip12'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip12
    MOV R0, #5                      ; R0 = 5
    BL  Fifo_Put                    ; FIFO:  [5] [ ] [ ] [ ] [ ]  5 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip13                      ; if not, skip to 'skip13'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip13
    LDR R0, num5Addr                ; R0 = &num5 (pointer)
    BL  Fifo_Get                    ; FIFO:  [ ] [ ] [ ] [ ] [ ]  5 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip14                      ; if not, skip to 'skip14'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip14
    LDR R0, num5Addr                ; R0 = &num5 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num5 (value)
    CMP R0, #5                      ; is R0 (num5) == 5? (was 5 correctly removed from the FIFO?)
    BEQ skip15                      ; if so, skip to 'skip15'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip15
;*************** second tests ***************
    MOV R0, #1                      ; R0 = 1
    BL  Fifo_Put                    ; FIFO:  [1] [ ] [ ] [ ] [ ]  1 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip16                      ; if not, skip to 'skip16'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip16
    MOV R0, #2                      ; R0 = 2
    BL  Fifo_Put                    ; FIFO:  [1] [2] [ ] [ ] [ ]  2 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip17                      ; if not, skip to 'skip17'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip17
    MOV R0, #3                      ; R0 = 3
    BL  Fifo_Put                    ; FIFO:  [1] [2] [3] [ ] [ ]  3 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip18                      ; if not, skip to 'skip18'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip18
    MOV R0, #4                      ; R0 = 4
    BL  Fifo_Put                    ; FIFO:  [1] [2] [3] [4] [ ]  4 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip19                      ; if not, skip to 'skip19'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip19
    MOV R0, #5                      ; R0 = 5
    BL  Fifo_Put                    ; FIFO:  [1] [2] [3] [4] [5]  5 added to FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip20                      ; if not, skip to 'skip20'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip20
    MOV R0, #6                      ; R0 = 6
    BL  Fifo_Put                    ; FIFO:  [1] [2] [3] [4] [5]  expect error; FIFO full
    CMP R0, #1                      ; is R0 == 1? (did the operation succeed?)
    BNE skip21                      ; if not, skip to 'skip21'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip21
    LDR R0, num1Addr                ; R0 = &num1 (pointer)
    BL  Fifo_Get                    ; FIFO:  [2] [3] [4] [5] [ ]  1 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip22                      ; if not, skip to 'skip22'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip22
    LDR R0, num1Addr                ; R0 = &num1 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num1 (value)
    CMP R0, #1                      ; is R0 (num1) == 1? (was 1 correctly removed from the FIFO?)
    BEQ skip23                      ; if so, skip to 'skip23'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip23
    LDR R0, num2Addr                ; R0 = &num2 (pointer)
    BL  Fifo_Get                    ; FIFO:  [3] [4] [5] [ ] [ ]  2 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip24                      ; if not, skip to 'skip24'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip24
    LDR R0, num2Addr                   ; R0 = &num2 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num2 (value)
    CMP R0, #2                      ; is R0 (num2) == 2? (was 2 correctly removed from the FIFO?)
    BEQ skip25                      ; if so, skip to 'skip25'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip25
    LDR R0, num3Addr                ; R0 = &num3 (pointer)
    BL  Fifo_Get                    ; FIFO:  [4] [5] [ ] [ ] [ ]  3 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip26                      ; if not, skip to 'skip26'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip26
    LDR R0, num3Addr                ; R0 = &num3 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num3 (value)
    CMP R0, #3                      ; is R0 (num3) == 3? (was 3 correctly removed from the FIFO?)
    BEQ skip27                      ; if so, skip to 'skip27'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip27
    LDR R0, num4Addr                ; R0 = &num4 (pointer)
    BL  Fifo_Get                    ; FIFO:  [5] [ ] [ ] [ ] [ ]  4 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip28                      ; if not, skip to 'skip28'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip28
    LDR R0, num4Addr                ; R0 = &num4 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num4 (value)
    CMP R0, #4                      ; is R0 (num4) == 4? (was 4 correctly removed from the FIFO?)
    BEQ skip29                      ; if so, skip to 'skip29'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip29
    LDR R0, num5Addr                ; R0 = &num5 (pointer)
    BL  Fifo_Get                    ; FIFO:  [ ] [ ] [ ] [ ] [ ]  5 removed from FIFO
    CMP R0, #0                      ; is R0 == 0? (did the operation fail?)
    BNE skip30                      ; if not, skip to 'skip30'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip30
    LDR R0, num5Addr                ; R0 = &num5 (pointer)
    LDR R0, [R0]                    ; R0 = [R0] = num5 (value)
    CMP R0, #5                      ; is R0 (num5) == 5? (was 5 correctly removed from the FIFO?)
    BEQ skip31                      ; if so, skip to 'skip31'
    BL  crash                       ; otherwise, run 'crash' subroutine
skip31
    LDR R0, num1Addr                ; R0 = &num1 (pointer)
    BL  Fifo_Get                    ; FIFO:  [ ] [ ] [ ] [ ] [ ]  expect error; FIFO empty
    CMP R0, #1                      ; is R0 == 1? (did the operation succeed?)
    BNE loop                        ; if not, skip to 'loop'
    BL  crash                       ; otherwise, run 'crash' subroutine
    B   loop                        ; unconditional branch to 'loop'
    .endasmfunc
    .align 4

num1Addr   .field num1,32
num2Addr   .field num2,32
num3Addr   .field num3,32
num4Addr   .field num4,32
num5Addr   .field num5,32
    .end                             ; end of file

