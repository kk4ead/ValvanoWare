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

        IMPORT  Fifo_Init
        IMPORT  Fifo_Put
        IMPORT  Fifo_Get
        IMPORT  Fifo_Size

        AREA    DATA, ALIGN=2
n       SPACE   8
        EXPORT n [DATA,SIZE=1]
        ALIGN
        
        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
Start
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
      LDR R0,=n
      BL  Fifo_Get
      LDR R0,=n+1
      BL  Fifo_Get
      LDR R0,=n+2
      BL  Fifo_Get
      LDR R0,=n+3
      BL  Fifo_Get
      LDR R0,=n+4
      BL  Fifo_Get
      LDR R0,=n+5
      BL  Fifo_Get
      LDR R0,=n+6
      BL  Fifo_Get
      LDR R0,=n+7
      BL  Fifo_Get  ;should fail
      
      B loop                          ; unconditional branch to 'loop'


    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
        