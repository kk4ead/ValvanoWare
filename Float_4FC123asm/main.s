; main.s
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
        AREA    DATA, ALIGN=2
ten     SPACE   4
tenth   SPACE   4
In      SPACE   4
A       SPACE   4   
T       SPACE   4
N       SPACE   4
y     SPACE   4  ; current filter output
y1    SPACE   4  ; filter output 1ms ago
y2    SPACE   4  ; filter output 2ms ago
x     SPACE   4  ; current filter input
x1    SPACE   4  ; input 1ms ago
x2    SPACE   4  ; input 2ms ago


Buffer  SPACE   80  ; results of circle test
        EXPORT ten [DATA,SIZE=4]
        EXPORT tenth [DATA,SIZE=4]
        EXPORT Buffer [DATA,SIZE=80]

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  Start
; FPU already initialized in this start.s
Start

Test0
     BL  Test4
     VLDR.F32 S0,=10.0 ;the example from the book
     LDR  R0,=ten
     VSTR.F32 S0,[R0]
; 10 = (0,0x82,0x200000) = (0,10000010,01000000000000000000000)
     VLDR.F32 S1,=0.1  ;another example from the book
     LDR  R0,=tenth
     VSTR.F32 S1,[R0]
; 0.1 about (0,0x7B,0x4CCCCD) = (0,01111011,10011001100110011001101)

     VLDR.F32 S2,=0.0 
     VLDR.F32 S3,=0.5 
     
     LDR  R0,=In
     VSTR.F32 S3,[R0] ;In = 0.5
     BL   Test
     VLDR.F32 S4,=0.785398  ;pi/4 
     
     LDR  R4,=Buffer
     MOV  R5,#20

loop 
     VMOV.F32 S0,S2   ; set input parameter
     BL   CircleArea 
     VSTR.F32 S0,[R4],#4
     VADD.F32 S2,S2,S3  ; next input
     SUBS R5,#1
     BNE  loop
done B    done

Test PUSH {LR}
     LDR  R0,=In
     VLDR.F32 S0,[R0] ;S0 is In
     BL   CircleArea
     LDR  R0,=A
     VSTR.F32 S0,[R0] ;A=pi*In*In
     POP  {PC}

; Calculate the area of a circle
; Input: S0 is the radius of the circle
; Output: S0 is the area of the circle
CircleArea
     VMUL.F32 S0,S0,S0  ; input squared
     VLDR.F32 S1,=3.14159265
     VMUL.F32 S0,S0,S1  ; pi*r*r
     BX  LR     
     
NVIC_CPAC_R equ 0xE000ED88
EnableFPU
      LDR R0, =NVIC_CPAC_R
      LDR R1, [R0] ; Read CPACR
      ORR R1, R1, #0x00F00000  ; Set bits 20-23 to enable CP10 and CP11 coprocessors
      STR R1, [R0]
      BX  LR


Test3
      MOV R0,#0
      LDR R1,=N
      LDR R2,=T
      VLDR.F32 S1,=0.009768   
      VLDR.F32 S2,=10   

loop3 STR R0,[R1]          ; N is volatile
      VMOV.F32 S0,R0
      VCVT.F32.U32 S0,S0   ; S0 has N
      VMUL.F32 S0,S0,S1    ;N*0.09768
      VADD.F32 S0,S0,S2    ;10+N*0.0968
      VSTR.F32 S0,[R2]     ;T=10+N*0.0968
      ADD R0,R0,#1
      CMP R0,#4096
      BNE loop3
      BX  LR
     
Test4
      MOV R0,#0
      LDR R1,=N
      LDR R2,=T
      MOV R3,#125   
      MOV R4,#64
      MOV R5,#1000   

loop4 STR R0,[R1]          ; N is volatile
      MUL R6,R0,R3         ; N*125
      ADD R6,R6,R4         ; N*125+64
      LSR R6,R6,#7         ; (N*125+64)/128
      ADD R6,R6,R5         ; 1000+(N*125+64)/128
      STR R6,[R2]          ; T = 1000+(N*125+64)/128
      ADD R0,R0,#1
      CMP R0,#4096
      BNE loop4
      BX  LR
     
; Input: S0 is new input
; Output: S0 is filter output
Notch60Hz
     LDR      R0,=x
     VLDR.F32 S1,[R0,#4] ;read previous x1
     VSTR.F32 S1,[R0,#8] ;S1 is x2
     VLDR.F32 S2,[R0,#0] ;read previous x
     VSTR.F32 S2,[R0,#4] ;S2 is x1
     VSTR.F32 S0,[R0,#0] ;S0 is x = in
     LDR      R1,=y
     VLDR.F32 S3,[R1,#4] ;read previous y1
     VSTR.F32 S3,[R1,#8] ;S3 is y2
     VLDR.F32 S4,[R1,#0] ;read previous y
     VSTR.F32 S4,[R1,#4] ;S4 is y1 
     VLDR.F32 S5,=-1.8595529717765
     VMUL.F32 S2,S2,S5 
     VADD.F32 S0,S0,S2 ;-1.8595529717765*x1 
     VADD.F32 S0,S0,S1 ;+x2 
     VLDR.F32 S5,=1.84095744205874
     VMUL.F32 S4,S4,S5 
     VADD.F32 S0,S0,S4;+1.84095744205874*y1 
     VLDR.F32 S5,=-0.9801
     VMUL.F32 S3,S3,S5 
     VADD.F32 S0,S0,S3 ; -0.9801*y2 
     VSTR.F32 S0,[R1,#0] ;set y 
     BX  LR 

Test5
      LDR R2,=y
      VLDR.F32 S6,=0.0   
      VSTR.F32 S6,[R2]
      VSTR.F32 S6,[R2,#4]
      VSTR.F32 S6,[R2,#8]
      VSTR.F32 S6,[R2,#12]
      VSTR.F32 S6,[R2,#16]
      VSTR.F32 S6,[R2,#20]
      
      VLDR.F32 S6,=100.0   
      VSTR.F32 S6,[R2]  

      LDR R2,=N
      VLDR.F32 S6,=100.0   
      VSTR.F32 S6,[R2]   

loop5 LDR R2,=N
      VLDR.F32 S0,[R2]   ;input
      BL Notch60Hz
      B   loop5
      BX  LR
    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
