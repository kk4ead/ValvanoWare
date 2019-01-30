; Heap.s
; Runs on any computer
; A fixed-block memory manager.
; Valvano
; April 22, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 6.10 through 6.13, Section 6.6
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

SIZE   EQU 4    ; number of 32-bit words in each block
NUM    EQU 5    ; number of blocks
NULL   EQU 0    ; definition of empty pointer

        AREA    DATA, ALIGN=4
FreePt  SPACE 4
Heap    SPACE SIZE*NUM*4
;export for use with debugger
        EXPORT FreePt [DATA,SIZE=4]

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Heap_Init
        EXPORT  Heap_Allocate
        EXPORT  Heap_Release

;------------Heap_Init------------
; Initialize the heap and pointer to the first free block.  Free
; blocks are linked together with a linear linked list
; implemented using the first 32-bit word of each free block to
; point to the next free block.  The last free block points to
; NULL to signify that there are no free blocks after it.
; Input: none
; Output: none
; Modifies: R0, R1, R2, R3
Heap_Init LDR  R0,=Heap
          LDR  R1,=FreePt
          STR  R0,[R1]       ; FreePt=&Heap[0];
          MOV  R2,#SIZE
          MOV  R3,#NUM-1
imLoop    ADD  R1,R0,#SIZE*4 ; pt+SIZE
          STR  R1,[R0]       ; *pt=pt+SIZE;
          MOV  R0,R1         ; pt=pt+SIZE;
          SUBS R3,R3,#1
          BNE  imLoop
          MOV  R1,#NULL
          STR  R1,[R0]       ; last ptr is NULL
          BX   LR

;------------Heap_Allocate------------
; Attempt to allocate a block, removing it from the free list.
; The pointer to the first free block is moved to the next free
; block after the block that is being allocated, which is now
; no longer free.
; If no blocks are free, this function returns a NULL pointer.
; Input: none
; Output: R0 pointer to free block or NULL if all blocks full
; Modifies: R1, R2
Heap_Allocate                ; R0 points to new block
      LDR R1,=FreePt
      LDR R0,[R1]            ; R0=FreePt;
      CMP R0,#NULL
      BEQ aDone              ; if (pt!=NULL)
      LDR R2,[R0]            ; link next
      STR R2,[R1]            ; FreePt=*pt;
aDone BX  LR

;------------Heap_Release------------
; Release a block, adding it to the free list, and making it
; available for future use.  The pointer to the first free block
; is moved to this block, and the next free block is the
; previous first free block.  In other words, the newly freed
; block is inserted at the beginning of the free list.
; IMPORTANT: This function does not check anything about the
; pointer and does not check if the block was previously
; allocated.
; An invalid pointer may produce unpredictable behavior.
; Input: R0  pointer to the block to be released
; Output: none
; Modifies: R1, R2
Heap_Release                 ; R0 => block being released
      LDR R1,=FreePt
      LDR R2,[R1]            ; R2=oldFreePt
      STR R0,[R1]            ; FreePt=pt
      STR R2,[R0]            ; *pt=oldFreePt;
      BX  LR

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
