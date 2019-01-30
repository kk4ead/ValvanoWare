; SysTick.asm
; Runs on LM4F120/TM4C123
; Provide functions that initialize the SysTick module, wait at least a
; designated number of clock cycles, and wait approximately a multiple
; of 10 milliseconds using busy wait.  After a power-on-reset, the
; LM4F120 gets its clock from the 16 MHz precision internal oscillator,
; which can vary by +/- 1% at room temperature and +/- 3% across all
; temperature ranges.  If you are using this module, you may need more
; precise timing, so it is assumed that you are using the PLL to set
; the system clock to 50 MHz.  This matters for the function
; SysTick_Wait10ms(), which will wait longer than 10 ms if the clock is
; slower.
; Daniel Valvano
; May 3, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Program 2.11, Section 2.6
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


NVIC_ST_CTRL_R        .field 0xE000E010,32
NVIC_ST_RELOAD_R      .field 0xE000E014,32
NVIC_ST_CURRENT_R     .field 0xE000E018,32
NVIC_ST_CTRL_COUNT    .equ 0x00010000  ; Count flag
NVIC_ST_CTRL_CLK_SRC  .equ 0x00000004  ; Clock Source
NVIC_ST_CTRL_INTEN    .equ 0x00000002  ; Interrupt enable
NVIC_ST_CTRL_ENABLE   .equ 0x00000001  ; Counter mode
NVIC_ST_RELOAD_M      .field 0x00FFFFFF,32  ; Counter load value


        .global   SysTick_Init
        .global   SysTick_Wait
        .global   SysTick_Wait10ms

;------------SysTick_Init------------
; Initialize SysTick with busy wait running at bus clock.
; Input: none
; Output: none
; Modifies: R0, R1
SysTick_Init:  .asmfunc
    ; disable SysTick during setup
    LDR R1, NVIC_ST_CTRL_R         ; R1 = &NVIC_ST_CTRL_R
    MOV R0, #0                      ; R0 = 0
    STR R0, [R1]                    ; [R1] = R0 = 0
    ; maximum reload value
    LDR R1, NVIC_ST_RELOAD_R       ; R1 = &NVIC_ST_RELOAD_R
    LDR R0, NVIC_ST_RELOAD_M;      ; R0 = NVIC_ST_RELOAD_M
    STR R0, [R1]                    ; [R1] = R0 = NVIC_ST_RELOAD_M
    ; any write to current clears it
    LDR R1, NVIC_ST_CURRENT_R      ; R1 = &NVIC_ST_CURRENT_R
    MOV R0, #0                      ; R0 = 0
    STR R0, [R1]                    ; [R1] = R0 = 0
    ; enable SysTick with core clock
    LDR R1, NVIC_ST_CTRL_R         ; R1 = &NVIC_ST_CTRL_R
                                    ; R0 = ENABLE and CLK_SRC bits set
    MOV R0, #(NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC)
    STR R0, [R1]                    ; [R1] = R0 = (NVIC_ST_CTRL_ENABLE|NVIC_ST_CTRL_CLK_SRC)
    BX  LR                          ; return
    .endasmfunc
;------------SysTick_Wait------------
; Time delay using busy wait.
; Input: R0  delay parameter in units of the core clock (units of 20 nsec for 50 MHz clock)
; Output: none
; Modifies: R1, R2, R3
SysTick_Wait:  .asmfunc
    ; get the starting time (R2)
    LDR R1, NVIC_ST_CURRENT_R      ; R1 = &NVIC_ST_CURRENT_R
    LDR R2, [R1]                    ; R2 = [R1] = startTime
SysTick_Wait_loop
    ; determine the elapsed time (R3)
    LDR R3, [R1]                    ; R3 = [R1] = currentTime
    SUB R3, R2, R3                  ; R3 = R2 - R3 = startTime - currentTime
    ; handle possible counter roll over by converting to 24-bit subtraction
    BIC R3, R3, #0xFF000000
    ; is elapsed time (R3) <= delay (R0)?
    CMP R3, R0
    BLS SysTick_Wait_loop
    BX  LR                          ; return
    .endasmfunc
;------------SysTick_Wait10ms------------
; Time delay using busy wait.  This assumes 50 MHz clock
; Input: R0  number of times to wait 10 ms before returning
; Output: none
; Modifies: R0
DELAY10MS  .field 500000,32    ; clock cycles in 10 ms (assumes 50 MHz clock)
SysTick_Wait10ms:  .asmfunc
    PUSH {R4, LR}                   ; save current value of R4 and LR
    MOVS R4, R0                     ; R4 = R0 = remainingWaits
    BEQ SysTick_Wait10ms_done       ; R4 == 0, done
SysTick_Wait10ms_loop
    LDR R0, DELAY10MS               ; R0 = DELAY10MS
    BL  SysTick_Wait                ; wait 10 ms
    SUBS R4, R4, #1                 ; R4 = R4 - 1; remainingWaits--
    BHI SysTick_Wait10ms_loop       ; if(R4 > 0), wait another 10 ms
SysTick_Wait10ms_done
    POP {R4, LR}                    ; restore previous value of R4 and LR
    BX  LR                          ; return
    .endasmfunc
    .end                            ; end of file
