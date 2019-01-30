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
       .thumb

       .data
       .align  2
ten    .space   4
tenth  .space   4
In     .space   4
A      .space   4
T      .space   4
N      .space   4
y      .space   4  ; current filter output
y1     .space   4  ; filter output 1ms ago
y2     .space   4  ; filter output 2ms ago
x      .space   4  ; current filter input
x1     .space   4  ; input 1ms ago
x2     .space   4  ; input 2ms ago

Buffer .space  80  ; results of circle test


       .text
       .align  2
      .global main
; FPU already initialized in this start.s
main:  .asmfunc

Test0
     BL  Test4
     VLDR.32 S0,f4 ; =10.0 ;the example from the book
     LDR  R0,tenAddr
     VSTR.32 S0,[R0]
; 10 = (0,0x82,0x200000) = (0,10000010,01000000000000000000000)
     VLDR.32 S1,f1  ;0.1 another example from the book
     LDR  R0,tenthAddr
     VSTR.32 S1,[R0]
; 0.1 about (0,0x7B,0x4CCCCD) = (0,01111011,10011001100110011001101)

     VLDR.32 S2,f0 ;0.0
     VLDR.32 S3,f2 ;0.5

     LDR  R0,InAddr
     VSTR.32 S3,[R0] ;In = 0.5
     BL   Test
     VLDR.32 S4,f3 ; 0.785398  ;pi/4

     LDR  R4,BufferAddr
     MOV  R5,#20

loop
     VMOV.F32 S0,S2   ; set input parameter
     BL   CircleArea
     VSTR.32 S0,[R4]
     ADD  R4,#4
     VADD.F32 S2,S2,S3  ; next input
     SUBS R5,#1
     BNE  loop
done B    done
    .endasmfunc
f1  .float 0.1
f0  .float 0.0
f2  .float 0.5
f3  .float 0.785398
f4  .float 10.0
Test:  .asmfunc
     PUSH {LR}
     LDR  R0,InAddr
     VLDR.32 S0,[R0] ;S0 is In
     BL   CircleArea
     LDR  R0,AAddr
     VSTR.32 S0,[R0] ;A=pi*In*In
     POP  {PC}
    .endasmfunc

; Calculate the area of a circle
; Input: S0 is the radius of the circle
; Output: S0 is the area of the circle
CircleArea:  .asmfunc
     VMUL.F32 S0,S0,S0  ; input squared
     VLDR.32 S1,pi
     VMUL.F32 S0,S0,S1  ; pi*r*r
     BX  LR
    .endasmfunc
pi  .float 3.14159265

EnableFPU:  .asmfunc
      LDR R0, NVIC_CPAC_R
      LDR R1, [R0] ; Read CPACR
      ORR R1, R1, #0x00F00000  ; Set bits 20-23 to enable CP10 and CP11 coprocessors
      STR R1, [R0]
      BX  LR
    .endasmfunc


Test3:  .asmfunc
      MOV R0,#0
      LDR R1,NAddr
      LDR R2,TAddr
      VLDR.32 S1,f5 ;0.009768
      VLDR.32 S2,f6 ;10

loop3 STR R0,[R1]          ; N is volatile
      VMOV    S0,R0
      VCVT.F32.U32 S0,S0   ; S0 has N
      VMUL.F32 S0,S0,S1    ;N*0.09768
      VADD.F32 S0,S0,S2    ;10+N*0.0968
      VSTR.32 S0,[R2]     ;T=10+N*0.0968
      ADD R0,R0,#1
      CMP R0,#4096
      BNE loop3
      BX  LR
    .endasmfunc
f5  .float 0.009768
f6  .float 10.0

Test4:  .asmfunc
      MOV R0,#0
      LDR R1,NAddr
      LDR R2,TAddr
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
    .endasmfunc

; Input: S0 is new input
; Output: S0 is filter output
Notch60Hz:  .asmfunc
     LDR      R0,xAddr
     VLDR.32 S1,[R0,#4] ;read previous x1
     VSTR.32 S1,[R0,#8] ;S1 is x2
     VLDR.32 S2,[R0,#0] ;read previous x
     VSTR.32 S2,[R0,#4] ;S2 is x1
     VSTR.32 S0,[R0,#0] ;S0 is x = in
     LDR      R1,yAddr
     VLDR.32 S3,[R1,#4] ;read previous y1
     VSTR.32 S3,[R1,#8] ;S3 is y2
     VLDR.32 S4,[R1,#0] ;read previous y
     VSTR.32 S4,[R1,#4] ;S4 is y1
     VLDR.32 S5,f7 ;-1.8595529717765
     VMUL.F32 S2,S2,S5
     VADD.F32 S0,S0,S2 ;-1.8595529717765*x1
     VADD.F32 S0,S0,S1 ;+x2
     VLDR.32 S5,f8 ;1.84095744205874
     VMUL.F32 S4,S4,S5
     VADD.F32 S0,S0,S4;+1.84095744205874*y1
     VLDR.32 S5,f9 ;-0.9801
     VMUL.F32 S3,S3,S5
     VADD.F32 S0,S0,S3 ; -0.9801*y2
     VSTR.32 S0,[R1,#0] ;set y
     BX  LR
    .endasmfunc
f7  .float -1.8595529717765
f8  .float 1.84095744205874
f9  .float -0.9801

Test5:  .asmfunc
      LDR R2,yAddr
      VLDR.32 S6,f0 ;0.0
      VSTR.32 S6,[R2]
      VSTR.32 S6,[R2,#4]
      VSTR.32 S6,[R2,#8]
      VSTR.32 S6,[R2,#12]
      VSTR.32 S6,[R2,#16]
      VSTR.32 S6,[R2,#20]

      VLDR.32 S6,f10 ;100.0
      VSTR.32 S6,[R2]

      LDR R2,NAddr
      VLDR.32 S6,f10 ;100.0
      VSTR.32 S6,[R2]

loop5 LDR R2,NAddr
      VLDR.32 S0,[R2]   ;input
      BL Notch60Hz
      B   loop5
      BX  LR
    .endasmfunc
f10 .float 100.0

tenAddr    .field    ten,32
tenthAddr  .field    tenth,32
InAddr     .field    In,32
AAddr      .field    A,32
TAddr      .field    T,32
NAddr      .field    N,32
yAddr      .field    y,32   ; current filter output
y1Addr     .field    y1,32  ; filter output 1ms ago
y2Addr     .field    y2,32  ; filter output 2ms ago
xAddr      .field    x,32   ; current filter input
x1Addr     .field    x1,32  ; input 1ms ago
x2Addr     .field    x2,32  ; input 2ms ago
BufferAddr .field    Buffer,32
NVIC_CPAC_R .field 0xE000ED88,32
    .end
