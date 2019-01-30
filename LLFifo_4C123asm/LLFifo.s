; LLFifo.s
; Runs on any computer
; linked list FIFO
; Jonathan Valvano
; April 23, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
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

        PRESERVE8
        AREA    DATA, ALIGN=2
;put in RAM
Next  EQU  0   ;next
Data  EQU  4   ;32-bit data for node
GetPt SPACE  4   ; GetPt  is pointer to oldest node
PutPt SPACE  4   ; PutPt is pointer to newest node
;normally GetPt and PutPt should be private, but exporting allows one to view in debugger
  EXPORT GetPt [DATA,SIZE=4]
  EXPORT PutPt [DATA,SIZE=4]

NULL  EQU  0
    ALIGN            ; make sure the end of this section is aligned

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Fifo_Init
        EXPORT  Fifo_Put
        EXPORT  Fifo_Get
        IMPORT  Heap_Init
        IMPORT  Heap_Allocate
        IMPORT  Heap_Release

Fifo_Init
    PUSH {LR}
    MOV  R1,#NULL
    LDR  R0,=GetPt
    STR  R1,[R0]    ;GetPt=NULL
    LDR  R0,=PutPt
    STR  R1,[R0]    ;PutPt=NULL
    BL   Heap_Init
    POP  {PC}


; Inputs:  R0 value, data to put
; Outputs: R0=1 if successful
;          R0=0 if unsuccessful
Fifo_Put
     PUSH {R4,LR}
     MOV  R4,R0   ;data to put
     BL   Heap_Allocate
     CMP  R0,#NULL
     BEQ  PFul     ;skip if full
     STR  R4,[R0,#Data] ;store data
     MOV  R1,#NULL
     STR  R1,[R0,#Next] ;next=NULL
     LDR  R2,=PutPt ;R2= &PutPt
     LDR  R3,[R2]   ;R3=PutPt
     CMP  R3,#NULL  ;previously MT?
     BEQ  PMT
     STR  R0,[R3,#Next] ;link previous
     B    PCon
PMT  LDR  R1,=GetPt ;R1= &GetPt
     STR  R0,[R1]   ;Now one entry
PCon STR  R0,[R2]   ;points to newest
     MOV  R0,#1     ;success
     B    PDon
PFul MOV  R0,#0     ;failure, full
PDon POP  {R4,PC}


; Inputs:  R0 points to an empty place
; Outputs: data removed to place
;          R0=0 if successful
;          R0=1 if empty
Fifo_Get
     PUSH {R4,LR}
     LDR  R1,=GetPt ;R1= &GetPt
     LDR  R2,[R1]   ;R2=GetPt
     CMP  R2,#NULL
     BEQ  GMT     ;empty if NULL
     LDR  R3,[R2,#Data]  ;read
     STR  R3,[R0] ;by reference
     LDR  R3,[R2,#Next] ;next link
     STR  R3,[R1] ; update GetPt
     CMP  R3,#NULL
     BNE  GCon
     LDR  R1,=PutPt   ;Now empty
     STR  R3,[R1]  ;update PutPt
GCon MOV  R0,R2    ;old data
     BL   Heap_Release
     MOV  R0,#1    ;success
     B    GDon
GMT  MOV  R0,#0    ;failure, empty
GDon POP  {R4,PC}

    ALIGN            ; make sure the end of this section is aligned
    END
