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

        EXPORT   TxFifo_Init
        EXPORT   TxFifo_Put
        EXPORT   TxFifo_Get
        EXPORT   TxFifo_Size
        EXPORT   RxFifo_Init
        EXPORT   RxFifo_Put
        EXPORT   RxFifo_Get
        EXPORT   RxFifo_Size
        
           AREA    DATA, ALIGN=2
SIZE       EQU     16
RxPutPt    SPACE   4
RxGetPt    SPACE   4
RxFifo     SPACE   SIZE  ; space for SIZE bytes
TxPutPt    SPACE   4
TxGetPt    SPACE   4
TxFifo     SPACE   SIZE  ; space for SIZE bytes

  
        
           AREA    |.text|, CODE, READONLY, ALIGN=2
           THUMB
;Initialize RxFifo
RxFifo_Init
           LDR R0,=RxFifo  ;pointer to beginning
           LDR R1,=RxPutPt
           STR R0,[R1]
           LDR R1,=RxGetPt
           STR R0,[R1]
           BX  LR
         
;Put data into RxFifo
; Input R0 8-bit data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was ful
RxFifo_Put LDR  R1,=RxPutPt  
           LDR  R2,[R1]         ;RxPutPt
           ADD  R3,R2,#1       
           LDR  R12,=RxFifo+SIZE
           CMP  R3,R12          ;check if should wrap
           BNE  NoWrap        
           LDR  R3,=RxFifo      ;wrap
NoWrap     LDR  R12,=RxGetPt     
           LDR  R12,[R12]       ;RxGetPt
           CMP  R3,R12          ;full when holding SIZE-1
           BNE  NotFull        
           MOV  R0,#0           ;full
           BX   LR             
NotFull    STRB R0,[R2]         ;save
           STR  R3,[R1]         ;update RxPutPt
           MOV  R0,#1           ;success
           BX   LR              

;remove one element from RxFifo
;Input: call by reference to a place to store removed data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was empty
RxFifo_Get PUSH {R4,R5,LR}
           LDR  R1,=RxPutPt     
           LDR  R1,[R1]    ;RxPutPt
           LDR  R2,=RxGetPt     
           LDR  R3,[R2]    ;RxGetPt
           CMP  R1,R3      ;empty if RxPutPt equals RxGetPt
           BNE  NotEmpty
           MOV  R0,#0      ;fail, empty
           B    done
NotEmpty   LDRSB R4,[R3]   ;read from RxFifo
           STRB  R4,[R0]   ;return by reference
           ADD  R3,R3,#1   ;next place to Get
           LDR  R5,=RxFifo+SIZE
           CMP  R3,R5      ;check if need to wrap
           BNE  NoWrap2          
           LDR  R3,=RxFifo ;wrap  
NoWrap2    STR  R3,[R2]    ;update RxGetPt
done       POP  {R4,R5,PC}

;Returns the number of elements in the RxFifo
; Input:  none
; Output: R0 0 to SIZE-1
RxFifo_Size 
           LDR  R1,=RxPutPt     
           LDR  R1,[R1]    ;RxPutPt
           LDR  R2,=RxGetPt     
           LDR  R3,[R2]    ;RxGetPt
           SUB  R0,R1,R3   ;RxPutPt-RxGetPt
           AND  R0,#(SIZE-1) ; SIZE must be a power of two
           BX   LR   

;Initialize TxFifo
TxFifo_Init
           LDR R0,=TxFifo  ;pointer to beginning
           LDR R1,=TxPutPt
           STR R0,[R1]
           LDR R1,=TxGetPt
           STR R0,[R1]
           BX  LR
         
;Put data into TxFifo
; Input R0 8-bit data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was ful
TxFifo_Put LDR  R1,=TxPutPt  
           LDR  R2,[R1]         ;TxPutPt
           ADD  R3,R2,#1       
           LDR  R12,=TxFifo+SIZE
           CMP  R3,R12          ;check if should wrap
           BNE  TxNoWrap        
           LDR  R3,=TxFifo      ;wrap
TxNoWrap   LDR  R12,=TxGetPt     
           LDR  R12,[R12]       ;TxGetPt
           CMP  R3,R12          ;full when holding SIZE-1
           BNE  TxNotFull        
           MOV  R0,#0           ;full
           BX   LR             
TxNotFull  STRB R0,[R2]         ;save
           STR  R3,[R1]         ;update TxPutPt
           MOV  R0,#1           ;success
           BX   LR              

;remove one element from TxFifo
;Input: call by reference to a place to store removed data
; Output: R0 1 if successful
;            0 if unsuccessful, because it was empty
TxFifo_Get PUSH {R4,R5,LR}
           LDR  R1,=TxPutPt     
           LDR  R1,[R1]    ;TxPutPt
           LDR  R2,=TxGetPt     
           LDR  R3,[R2]    ;TxGetPt
           CMP  R1,R3      ;empty if TxPutPt equals TxGetPt
           BNE  TxNotEmpty
           MOV  R0,#0      ;fail, empty
           B    Txdone
TxNotEmpty LDRSB R4,[R3]   ;read from TxFifo
           STRB  R4,[R0]   ;return by reference
           ADD  R3,R3,#1   ;next place to Get
           LDR  R5,=TxFifo+SIZE
           CMP  R3,R5      ;check if need to wrap
           BNE  TxNoWrap2          
           LDR  R3,=TxFifo ;wrap  
TxNoWrap2  STR  R3,[R2]    ;update TxGetPt
Txdone     POP  {R4,R5,PC}

;Returns the number of elements in the TxFifo
; Input:  none
; Output: R0 0 to SIZE-1
TxFifo_Size 
           LDR  R1,=TxPutPt     
           LDR  R1,[R1]    ;TxPutPt
           LDR  R2,=TxGetPt     
           LDR  R3,[R2]    ;TxGetPt
           SUB  R0,R1,R3   ;TxPutPt-TxGetPt
           AND  R0,#(SIZE-1) ; SIZE must be a power of two
           BX   LR   
         
           ALIGN    ; make sure the end of this section is aligned
           END      ; end of file
        