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

         AREA    DATA, ALIGN=2
SIZE     EQU     8   ;must be a power of 2
PutPt    SPACE   4
GetPt    SPACE   4
Fifo     SPACE   SIZE  ; space for SIZE bytes
         EXPORT  PutPt [DATA,SIZE=4]
         EXPORT  GetPt [DATA,SIZE=4]
         EXPORT  Fifo [DATA,SIZE=8]
         EXPORT  Fifo_Init
         EXPORT  Fifo_Put
         EXPORT  Fifo_Get
         EXPORT  Fifo_Size 
        
         AREA    |.text|, CODE, READONLY, ALIGN=2
         THUMB
;Initialize FIFO
Fifo_Init
         LDR R0,=Fifo  ;pointer to beginning
         LDR R1,=PutPt
         STR R0,[R1]
         LDR R1,=GetPt
         STR R0,[R1]
         BX  LR
         
;Put data into FIFO
; Input R0 8-bit data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was ful
Fifo_Put LDR  R1,=PutPt  
         LDR  R2,[R1]         ;PutPt
         ADD  R3,R2,#1       
         LDR  R12,=Fifo+SIZE
         CMP  R3,R12          ;check if should wrap
         BNE  NoWrap        
         LDR  R3,=Fifo        ;wrap
NoWrap   LDR  R12,=GetPt     
         LDR  R12,[R12]       ;GetPt
         CMP  R3,R12          ;full when holding SIZE-1
         BNE  NotFull        
         MOV  R0,#0           ;full
         BX   LR             
NotFull  STRB R0,[R2]         ;save
         STR  R3,[R1]         ;update PutPt
         MOV  R0,#1           ;success
         BX   LR              

;remove one element from FIFO
;Input: call by reference to a place to store removed data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was empty
Fifo_Get PUSH {R4,R5,LR}
         LDR  R1,=PutPt     
         LDR  R1,[R1]    ;PutPt
         LDR  R2,=GetPt     
         LDR  R3,[R2]    ;GetPt
         CMP  R1,R3      ;empty if PutPt equals GetPt
         BNE  NotEmpty
         MOV  R0,#0      ;fail, empty
         B    done
NotEmpty LDRSB R4,[R3]   ;read from FIFO
         STRB  R4,[R0]   ;return by reference
         ADD  R3,R3,#1   ;next place to Get
         LDR  R5,=Fifo+SIZE
         CMP  R3,R5      ;check if need to wrap
         BNE  NoWrap2          
         LDR  R3,=Fifo   ;wrap  
NoWrap2  STR  R3,[R2]    ;update GetPt
done     POP  {R4,R5,PC}
         
;Returns the number of elements in the FIFO
; Input:  none
; Output: R0 0 to SIZE-1
Fifo_Size 
         LDR  R1,=PutPt     
         LDR  R1,[R1]    ;PutPt
         LDR  R2,=GetPt     
         LDR  R3,[R2]    ;GetPt
         SUB  R0,R1,R3   ;PutPt-GetPt
         AND  R0,#(SIZE-1) ; SIZE must be a power of two
         BX   LR   
         
         ALIGN           ; make sure the end of this section is aligned
         END             ; end of file
        