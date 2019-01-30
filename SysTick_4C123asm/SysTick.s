; SysTick.s
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
; September 12, 2013

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
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

NVIC_ST_CTRL_R        EQU 0xE000E010
NVIC_ST_RELOAD_R      EQU 0xE000E014
NVIC_ST_CURRENT_R     EQU 0xE000E018
NVIC_ST_CTRL_COUNT    EQU 0x00010000  ; Count flag
NVIC_ST_CTRL_CLK_SRC  EQU 0x00000004  ; Clock Source
NVIC_ST_CTRL_INTEN    EQU 0x00000002  ; Interrupt enable
NVIC_ST_CTRL_ENABLE   EQU 0x00000001  ; Counter mode
NVIC_ST_RELOAD_M      EQU 0x00FFFFFF  ; Counter load value

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT   SysTick_Init
        EXPORT   SysTick_Wait
        EXPORT   SysTick_Wait10ms

;------------SysTick_Init------------
; Initialize SysTick with busy wait running at bus clock.
; Input: none
; Output: none
; Modifies: R0, R1
SysTick_Init
    ; disable SysTick during setup
    LDR R1, =NVIC_ST_CTRL_R         ; R1 = &NVIC_ST_CTRL_R
    MOV R0, #0                      ; R0 = 0
    STR R0, [R1]                    ; [R1] = R0 = 0
    ; maximum reload value
    LDR R1, =NVIC_ST_RELOAD_R       ; R1 = &NVIC_ST_RELOAD_R
    LDR R0, =NVIC_ST_RELOAD_M;      ; R0 = NVIC_ST_RELOAD_M
    STR R0, [R1]                    ; [R1] = R0 = NVIC_ST_RELOAD_M
    ; any write to current clears it
    LDR R1, =NVIC_ST_CURRENT_R      ; R1 = &NVIC_ST_CURRENT_R
    MOV R0, #0                      ; R0 = 0
    STR R0, [R1]                    ; [R1] = R0 = 0
    ; enable SysTick with core clock
    LDR R1, =NVIC_ST_CTRL_R         ; R1 = &NVIC_ST_CTRL_R
                                    ; R0 = ENABLE and CLK_SRC bits set
    MOV R0, #(NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC)
    STR R0, [R1]                    ; [R1] = R0 = (NVIC_ST_CTRL_ENABLE|NVIC_ST_CTRL_CLK_SRC)
    BX  LR                          ; return

;------------SysTick_Wait------------
; Time delay using busy wait.
; Input: R0  delay parameter in units of the core clock (units of 12.5 nsec for 80 MHz clock)
; Output: none
; Modifies: R0, R1, R3
SysTick_Wait
    LDR  R1, =NVIC_ST_RELOAD_R      ; R1 = &NVIC_ST_RELOAD_R
    SUB  R0, #1
    STR  R0, [R1]                   ;delay-1;  // number of counts to wait
    LDR  R1, =NVIC_ST_CTRL_R        ; R1 = &NVIC_ST_CTRL_R
SysTick_Wait_loop
    LDR  R3, [R1]                   ; R3 = NVIC_ST_CTRL_R
    ANDS R3, R3, #0x00010000       ; Count set?
    BEQ  SysTick_Wait_loop
    BX   LR                         ; return

;------------SysTick_Wait10ms------------
; Time delay using busy wait.  This assumes 50 MHz clock
; Input: R0  number of times to wait 10 ms before returning
; Output: none
; Modifies: R0
DELAY10MS             EQU 800000    ; clock cycles in 10 ms (assumes 80 MHz clock)
SysTick_Wait10ms
    PUSH {R4, LR}                   ; save current value of R4 and LR
    MOVS R4, R0                     ; R4 = R0 = remainingWaits
    BEQ SysTick_Wait10ms_done       ; R4 == 0, done
SysTick_Wait10ms_loop
    LDR R0, =DELAY10MS              ; R0 = DELAY10MS
    BL  SysTick_Wait                ; wait 10 ms
    SUBS R4, R4, #1                 ; R4 = R4 - 1; remainingWaits--
    BHI SysTick_Wait10ms_loop       ; if(R4 > 0), wait another 10 ms
SysTick_Wait10ms_done
    POP {R4, LR}                    ; restore previous value of R4 and LR
    BX  LR                          ; return

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
