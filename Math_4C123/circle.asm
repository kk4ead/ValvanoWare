; circle.asm
; Runs on LM4F120/TM4C123
; Provide a brief introduction to the floating point unit
; Jonathan Valvano
; May 3, 2015

;  This example accompanies the book
;   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015

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
        .text
        .align 2

     .global CircleArea
; FPU already initialized in this start.s

; Calculate the area of a circle
; Input: S0 is the radius of the circle
; Output: S0 is the area of the circle
CircleArea:  .asmfunc
     VMUL.F32 S0,S0,S0  ; input squared
     VLDR.32  S1,pi     ; 3.14159
     VMUL.F32 S0,S0,S1  ; pi*r*r
     BX  LR
    .endasmfunc
pi  .float 3.14159
    .end                             ; end of file
