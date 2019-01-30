; FIFO.s
; Runs on LM4F120/TM4C123
; assembly language FIFO
; Jonathan Valvano
; May 3, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
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

SIZE     .equ     8   ;must be a power of 2
PutPt    .space   4
GetPt    .space   4
Fifo     .space   SIZE  ; space for SIZE bytes


       .text
       .align  2

;Initialize FIFO
      .global Fifo_Init
      .global Fifo_Put
      .global Fifo_Get
      .global Fifo_Size
Fifo_Init:  .asmfunc
         LDR R0,FifoAddr  ;pointer to beginning
         LDR R1,PutPtAddr
         STR R0,[R1]
         LDR R1,GetPtAddr
         STR R0,[R1]
         BX  LR
    .endasmfunc
;Put data into FIFO
; Input R0 8-bit data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was full
Fifo_Put:  .asmfunc
         LDR  R1,PutPtAddr
         LDR  R2,[R1]         ;PutPt
         ADD  R3,R2,#1
         LDR  R12,FifoEndAddr
         CMP  R3,R12          ;check if should wrap
         BNE  NoWrap
         LDR  R3,FifoAddr     ;wrap
NoWrap   LDR  R12,GetPtAddr
         LDR  R12,[R12]       ;GetPt
         CMP  R3,R12          ;full when holding SIZE-1
         BNE  NotFull
         MOV  R0,#0           ;full
         BX   LR
NotFull  STRB R0,[R2]         ;save
         STR  R3,[R1]         ;update PutPt
         MOV  R0,#1           ;success
         BX   LR
    .endasmfunc
;remove one element from FIFO
;Input: call by reference to a place to store removed data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was empty
Fifo_Get:  .asmfunc
         PUSH {R4,R5,LR}
         LDR  R1,PutPtAddr
         LDR  R1,[R1]    ;PutPt
         LDR  R2,GetPtAddr
         LDR  R3,[R2]    ;GetPt
         CMP  R1,R3      ;empty if PutPt equals GetPt
         BNE  NotEmpty
         MOV  R0,#0      ;fail, empty
         B    done
NotEmpty LDRSB R4,[R3]   ;read from FIFO
         STRB  R4,[R0]   ;return by reference
         ADD  R3,R3,#1   ;next place to Get
         LDR  R5,FifoEndAddr
         CMP  R3,R5      ;check if need to wrap
         BNE  NoWrap2
         LDR  R3,FifoAddr ;wrap
NoWrap2  STR  R3,[R2]    ;update GetPt
done     POP  {R4,R5,PC}
    .endasmfunc
;Returns the number of elements in the FIFO
; Input:  none
; Output: R0 0 to SIZE-1

Fifo_Size:  .asmfunc
         LDR  R1,PutPtAddr
         LDR  R1,[R1]    ;PutPt
         LDR  R2,GetPtAddr
         LDR  R3,[R2]    ;GetPt
         SUB  R0,R1,R3   ;PutPt-GetPt
         AND  R0,#(SIZE-1) ; SIZE must be a power of two
         BX   LR
    .endasmfunc
FifoAddr    .field Fifo,32   ; Address of Fifo
FifoEndAddr .field Fifo+SIZE,32   ; Address of Fifo
PutPtAddr   .field PutPt,32  ; Address of PutPt
GetPtAddr   .field GetPt,32  ; Address of GetPt
    .end
