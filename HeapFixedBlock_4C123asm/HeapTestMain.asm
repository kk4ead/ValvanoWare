; HeapTestMain.asm
; Runs on any computer
; Test the heap functions by continually allocating and releasing
; memory to ensure that the memory manager never crashes.
; Valvano
; May 3, 2015

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
        .thumb
        .data
        .align 2
Pt1     .space 4
Pt2     .space 4
Pt3     .space 4
Pt4     .space 4
Pt5     .space 4
Count   .space 4


        .text
        .align  2
        .global main
        .global  Heap_Init
        .global  Heap_Allocate
        .global  Heap_Release

main:  .asmfunc
    LDR R1, CountAddr               ; R1 = &Count
    MOV R0, #0                      ; R0 = 0
    STR R0, [R1]                    ; [R1] = R0 = 0 (Count = 0)
    BL  Heap_Init                   ; initialize the heap
    ; Allocate Pt1
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, Pt1Addr                 ; R1 = &Pt1
    STR R0, [R1]                    ; [R1] = R0 (Pt1 = R0)
loop
    ; Allocate Pt2
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, Pt2Addr                 ; R1 = &Pt2
    STR R0, [R1]                    ; [R1] = R0 (Pt2 = R0)
    ; Allocate Pt3
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, Pt3Addr                 ; R1 = &Pt3
    STR R0, [R1]                    ; [R1] = R0 (Pt3 = R0)
    ; Allocate Pt4
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, Pt4Addr                 ; R1 = &Pt4
    STR R0, [R1]                    ; [R1] = R0 (Pt4 = R0)
    ; Allocate Pt5
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, Pt5Addr                 ; R1 = &Pt5
    STR R0, [R1]                    ; [R1] = R0 (Pt5 = R0)
    ; Release Pt4
    LDR R0, Pt4Addr                 ; R0 = &Pt4 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt4 is no longer meaningful
    ; Release Pt3
    LDR R0, Pt3Addr                 ; R0 = &Pt3 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt3 is no longer meaningful
    ; Release Pt2
    LDR R0, Pt2Addr                 ; R0 = &Pt2 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt2 is no longer meaningful
    ; Release Pt5
    LDR R0, Pt5Addr                 ; R0 = &Pt5 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt5 is no longer meaningful
    LDR R1, CountAddr               ; R1 = &Count
    LDR R0, [R1]                    ; R0 = [R1] = Count
    ADD R0, R0, #1                  ; R0 = R0 + 1 (Count + 1)
    STR R0, [R1]                    ; [R1] = R0 (Count = Count + 1)
    B   loop
    .endasmfunc
Pt1Addr     .field Pt1,32
Pt2Addr     .field Pt2,32
Pt3Addr     .field Pt3,32
Pt4Addr     .field Pt4,32
Pt5Addr     .field Pt5,32
CountAddr   .field Count,32
    .end                             ; end of file
