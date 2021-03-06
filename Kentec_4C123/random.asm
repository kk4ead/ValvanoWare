;Random number generator;
; Linear congruential generator
; from Numerical Recipes by Press et al.
; Jonathan Valvano

; How to use:
; 1) call Random_Init once with a seed
;     Random_Init(1);
;     Random_Init(NVIC_CURRENT_R);
; 2) call Random over and over to get a new random number
;   n = Random();    // 32 bit number
;   m = Random()%60; // a number from 0 to 59

       .thumb
       .data
       .align 2

M      .space   4
       .text

       .global  Random_Init
       .global  Random
Random_Init:  .asmfunc
       LDR R2,MAddr    ; R4 = &M, R4 points to M
       MOV R0,#1       ; Initial seed
       STR R0,[R2]     ; M=1
       BX  LR
    .endasmfunc
MAddr .field  M,32
K1    .field  1664525,32
K2    .field  1013904223,32
;------------Random------------
; Return R0= random number
; Linear congruential generator
; from Numerical Recipes by Press et al.
Random:  .asmfunc
       LDR R2,MAddr ; R2 = &M, R4 points to M
       LDR R0,[R2]  ; R0=M
       LDR R1,K1    ; 1664525
       MUL R0,R0,R1 ; R0 = 1664525*M
       LDR R1,K2    ; 1013904223
       ADD R0,R1    ; 1664525*M+1013904223
       STR R0,[R2]  ; store M
       BX  LR
    .endasmfunc
    .end

