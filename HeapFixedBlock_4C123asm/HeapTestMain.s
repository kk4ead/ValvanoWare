; HeapTestMain.s
; Runs on any computer
; Test the heap functions by continually allocating and releasing
; memory to ensure that the memory manager never crashes.
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

        AREA    DATA, ALIGN=4
Pt1     SPACE 4
Pt2     SPACE 4
Pt3     SPACE 4
Pt4     SPACE 4
Pt5     SPACE 4
Count   SPACE 4
;export for use with debugger
        EXPORT Pt1   [DATA,SIZE=4]
        EXPORT Pt2   [DATA,SIZE=4]
        EXPORT Pt3   [DATA,SIZE=4]
        EXPORT Pt4   [DATA,SIZE=4]
        EXPORT Pt5   [DATA,SIZE=4]
        EXPORT Count [DATA,SIZE=4]

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        IMPORT  Heap_Init
        IMPORT  Heap_Allocate
        IMPORT  Heap_Release
        EXPORT  Start
Start
    LDR R1, =Count                  ; R1 = &Count
    MOV R0, #0                      ; R0 = 0
    STR R0, [R1]                    ; [R1] = R0 = 0 (Count = 0)
    BL  Heap_Init                   ; initialize the heap
    ; Allocate Pt1
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, =Pt1                    ; R1 = &Pt1
    STR R0, [R1]                    ; [R1] = R0 (Pt1 = R0)
loop
    ; Allocate Pt2
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, =Pt2                    ; R1 = &Pt2
    STR R0, [R1]                    ; [R1] = R0 (Pt2 = R0)
    ; Allocate Pt3
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, =Pt3                    ; R1 = &Pt3
    STR R0, [R1]                    ; [R1] = R0 (Pt3 = R0)
    ; Allocate Pt4
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, =Pt4                    ; R1 = &Pt4
    STR R0, [R1]                    ; [R1] = R0 (Pt4 = R0)
    ; Allocate Pt5
    BL  Heap_Allocate               ; R0 = &FreePt
    LDR R1, =Pt5                    ; R1 = &Pt5
    STR R0, [R1]                    ; [R1] = R0 (Pt5 = R0)
    ; Release Pt4
    LDR R0, =Pt4                    ; R0 = &Pt4 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt4 is no longer meaningful
    ; Release Pt3
    LDR R0, =Pt3                    ; R0 = &Pt3 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt3 is no longer meaningful
    ; Release Pt2
    LDR R0, =Pt2                    ; R0 = &Pt2 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt2 is no longer meaningful
    ; Release Pt5
    LDR R0, =Pt5                    ; R0 = &Pt5 (R0 points to pointer)
    LDR R0, [R0]                    ; R0 = [R0] = Pt4 (R0 gets contents of pointer)
    BL  Heap_Release                ; Pt5 is no longer meaningful
    LDR R1, =Count                  ; R1 = &Count
    LDR R0, [R1]                    ; R0 = [R1] = Count
    ADD R0, R0, #1                  ; R0 = R0 + 1 (Count + 1)
    STR R0, [R1]                    ; [R1] = R0 (Count = Count + 1)
    B   loop

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file