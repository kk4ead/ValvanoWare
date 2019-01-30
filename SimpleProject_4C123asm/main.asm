; main.s
; Runs on any Cortex M processor
; A very simple first project implementing a random number generator
; Daniel Valvano
; May 12, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Section 3.3.10, Program 3.12
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


; we align 32 bit variables to 32-bits
; we align op codes to 16 bits
       .thumb
       .data
       .align 2
M      .space 4

       .text
       .align 2
       .global  main
main:  .asmfunc
       LDR R2,MAddr    ; R2 = &M, R2 points to M
       MOV R0,#1       ; Initial seed
       STR R0,[R2]     ; M=1
loop   BL  Random
       B   loop
       .endasmfunc

;------------Random------------
; Return R0= random number
; Linear congruential generator
; from Numerical Recipes by Press et al.
Random:  .asmfunc
       LDR R2,MAddr ; R2 = &M, R2 points to M
       LDR R0,[R2]  ; R0=M
       LDR R1,k1
       MUL R0,R0,R1 ; R0 = 1664525*M
       LDR R1,k2
       ADD R0,R1    ; 1664525*M+1013904223
       STR R0,[R2]  ; store M
       BX  LR
       .endasmfunc
MAddr  .field M,32   ; Address of M
k1     .field 1664525,32
k2     .field 1013904223,32
       .end
