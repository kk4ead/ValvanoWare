; FIFO.s
; Runs on LM4F120/TM4C123
; main program to test the FIFO
; Jonathan Valvano
; September 11, 2013

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
   .align 2
        .global   TxFifo_Init
        .global   TxFifo_Put
        .global   TxFifo_Get
        .global   TxFifo_Size
        .global   RxFifo_Init
        .global   RxFifo_Put
        .global   RxFifo_Get
        .global   RxFifo_Size


SIZE       .equ     16
RxPutPt    .space   4
RxGetPt    .space   4
RxFifo     .space   SIZE  ; space for SIZE bytes
TxPutPt    .space   4
TxGetPt    .space   4
TxFifo     .space   SIZE  ; space for SIZE bytes


   .thumb
   .text
   .align 2
;Initialize RxFifo
RxFifo_Init:  .asmfunc
           LDR R0,RxFifoAddr  ;pointer to beginning
           LDR R1,RxPutPtAddr
           STR R0,[R1]
           LDR R1,RxGetPtAddr
           STR R0,[R1]
           BX  LR
   .endasmfunc
   .align 4
RxFifoAddr     .field RxFifo,32
RxFifoEndAddr  .field RxFifo+SIZE,32
RxPutPtAddr    .field RxPutPt,32
RxGetPtAddr    .field RxGetPt,32

;Put data into RxFifo
; Input R0 8-bit data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was ful
RxFifo_Put:  .asmfunc
           LDR  R1,RxPutPtAddr
           LDR  R2,[R1]         ;RxPutPt
           ADD  R3,R2,#1
           LDR  R12,RxFifoEndAddr
           CMP  R3,R12          ;check if should wrap
           BNE  NoWrap
           LDR  R3,RxFifoAddr      ;wrap
NoWrap     LDR  R12,RxGetPtAddr
           LDR  R12,[R12]       ;RxGetPt
           CMP  R3,R12          ;full when holding SIZE-1
           BNE  NotFull
           MOV  R0,#0           ;full
           BX   LR
NotFull    STRB R0,[R2]         ;save
           STR  R3,[R1]         ;update RxPutPt
           MOV  R0,#1           ;success
           BX   LR
   .endasmfunc

;remove one element from RxFifo
;Input: call by reference to a place to store removed data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was empty
RxFifo_Get:  .asmfunc
           PUSH {R4,R5,LR}
           LDR  R1,RxPutPtAddr
           LDR  R1,[R1]    ;RxPutPt
           LDR  R2,RxGetPtAddr
           LDR  R3,[R2]    ;RxGetPt
           CMP  R1,R3      ;empty if RxPutPt equals RxGetPt
           BNE  NotEmpty
           MOV  R0,#0      ;fail, empty
           B    done
NotEmpty   LDRSB R4,[R3]   ;read from RxFifo
           STRB  R4,[R0]   ;return by reference
           ADD  R3,R3,#1   ;next place to Get
           LDR  R5,RxFifoEndAddr
           CMP  R3,R5      ;check if need to wrap
           BNE  NoWrap2
           LDR  R3,RxFifoAddr ;wrap
NoWrap2    STR  R3,[R2]    ;update RxGetPt
done       POP  {R4,R5,PC}
   .endasmfunc

;Returns the number of elements in the RxFifo
; Input:  none
; Output: R0 0 to SIZE-1
RxFifo_Size:  .asmfunc
           LDR  R1,RxPutPtAddr
           LDR  R1,[R1]    ;RxPutPt
           LDR  R2,RxGetPtAddr
           LDR  R3,[R2]    ;RxGetPt
           SUB  R0,R1,R3   ;RxPutPt-RxGetPt
           AND  R0,#(SIZE-1) ; SIZE must be a power of two
           BX   LR
   .endasmfunc

;Initialize TxFifo
TxFifo_Init:  .asmfunc
           LDR R0,TxFifoAddr  ;pointer to beginning
           LDR R1,TxPutPtAddr
           STR R0,[R1]
           LDR R1,TxGetPtAddr
           STR R0,[R1]
           BX  LR
   .endasmfunc
   .align 4
TxFifoAddr     .field TxFifo,32
TxFifoEndAddr  .field TxFifo+SIZE,32
TxPutPtAddr    .field TxPutPt,32
TxGetPtAddr    .field TxGetPt,32
;Put data into TxFifo
; Input R0 8-bit data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was ful
TxFifo_Put:  .asmfunc
           LDR  R1,TxPutPtAddr
           LDR  R2,[R1]         ;TxPutPt
           ADD  R3,R2,#1
           LDR  R12,TxFifoEndAddr
           CMP  R3,R12          ;check if should wrap
           BNE  TxNoWrap
           LDR  R3,TxFifoAddr      ;wrap
TxNoWrap   LDR  R12,TxGetPtAddr
           LDR  R12,[R12]       ;TxGetPt
           CMP  R3,R12          ;full when holding SIZE-1
           BNE  TxNotFull
           MOV  R0,#0           ;full
           BX   LR
TxNotFull  STRB R0,[R2]         ;save
           STR  R3,[R1]         ;update TxPutPt
           MOV  R0,#1           ;success
           BX   LR
   .endasmfunc

;remove one element from TxFifo
;Input: call by reference to a place to store removed data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was empty
TxFifo_Get:  .asmfunc
           PUSH {R4,R5,LR}
           LDR  R1,TxPutPtAddr
           LDR  R1,[R1]    ;TxPutPt
           LDR  R2,TxGetPtAddr
           LDR  R3,[R2]    ;TxGetPt
           CMP  R1,R3      ;empty if TxPutPt equals TxGetPt
           BNE  TxNotEmpty
           MOV  R0,#0      ;fail, empty
           B    Txdone
TxNotEmpty LDRSB R4,[R3]   ;read from TxFifo
           STRB  R4,[R0]   ;return by reference
           ADD  R3,R3,#1   ;next place to Get
           LDR  R5,TxFifoEndAddr
           CMP  R3,R5      ;check if need to wrap
           BNE  TxNoWrap2
           LDR  R3,TxFifoAddr ;wrap
TxNoWrap2  STR  R3,[R2]    ;update TxGetPt
Txdone     POP  {R4,R5,PC}
   .endasmfunc

;Returns the number of elements in the TxFifo
; Input:  none
; Output: R0 0 to SIZE-1
TxFifo_Size:  .asmfunc
           LDR  R1,TxPutPtAddr
           LDR  R1,[R1]    ;TxPutPt
           LDR  R2,TxGetPtAddr
           LDR  R3,[R2]    ;TxGetPt
           SUB  R0,R1,R3   ;TxPutPt-TxGetPt
           AND  R0,#(SIZE-1) ; SIZE must be a power of two
           BX   LR
   .endasmfunc

           .end      ; end of file
