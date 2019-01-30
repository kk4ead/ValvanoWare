; Testmain.s
; Runs on LM4F120/TM4C123
; main program to test the FIFO
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
n:     .space   8

       .text
       .align  2
      .global main
      .global Fifo_Init
      .global Fifo_Put
      .global Fifo_Get
      .global Fifo_Size
      .thumbfunc main
main:  .asmfunc
      BL Fifo_Init
loop  MOV R0,#1
      BL  Fifo_Put
      MOV R0,#2
      BL  Fifo_Put
      MOV R0,#3
      BL  Fifo_Put
      MOV R0,#4
      BL  Fifo_Put
      MOV R0,#5
      BL  Fifo_Put
      MOV R0,#6
      BL  Fifo_Put
      MOV R0,#7
      BL  Fifo_Put
      MOV R0,#8
      BL  Fifo_Put  ;should fail
      BL  Fifo_Size ;should be 7
      LDR R0,nAddr0
      BL  Fifo_Get
      LDR R0,nAddr1
      BL  Fifo_Get
      LDR R0,nAddr2
      BL  Fifo_Get
      LDR R0,nAddr3
      BL  Fifo_Get
      LDR R0,nAddr4
      BL  Fifo_Get
      LDR R0,nAddr5
      BL  Fifo_Get
      LDR R0,nAddr6
      BL  Fifo_Get
      LDR R0,nAddr7
      BL  Fifo_Get  ;should fail

      B loop                          ; unconditional branch to 'loop'
    .endasmfunc
nAddr0  .field n,32     ; Address of n[0]
nAddr1  .field n+1,32   ; Address of n[1]
nAddr2  .field n+2,32   ; Address of n[2]
nAddr3  .field n+3,32   ; Address of n[3]
nAddr4  .field n+4,32   ; Address of n[4]
nAddr5  .field n+5,32   ; Address of n[5]
nAddr6  .field n+6,32   ; Address of n[6]
nAddr7  .field n+1,32   ; Address of n[7]

    .end
