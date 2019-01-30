; Switch.asm
; Runs on LM4F120/TM4C123
; Provide functions that initialize a GPIO as an input pin and
; allow reading of a positive logic switch.
; Use bit-banded I/O.
; Daniel and Jonathan Valvano
; September 12, 2013

; This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers"
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Section 4.2.2, Program 4.2, Figure 4.9

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

; negative logic switches connected to PF0 and PF4 on the Launchpad
; red LED connected to PF1 on the Launchpad
; blue LED connected to PF2 on the Launchpad
; green LED connected to PF3 on the Launchpad
; NOTE: The NMI (non-maskable interrupt) is on PF0.  That means that
; the Alternate Function Select, Pull-Up Resistor, Pull-Down Resistor,
; and Digital Enable are all locked for PF0 until a value of 0x4C4F434B
; is written to the Port F GPIO Lock Register.  After Port F is
; unlocked, bit 0 of the Port F GPIO Commit Register must be set to
; allow access to PF0's control registers.  On the LM4F120, the other
; bits of the Port F GPIO Commit Register are hard-wired to 1, meaning
; that the rest of Port F can always be freely re-configured at any
; time.  Requiring this procedure makes it unlikely to accidentally
; re-configure the JTAG and NMI pins as GPIO, which can lock the
; debugger out of the processor and make it permanently unable to be
; debugged or re-programmed.
                   .thumb
                   .text
                   .align 2
GPIO_PORTA_DIR_R   .field 0x40004400,32
GPIO_PORTA_AFSEL_R .field 0x40004420,32
GPIO_PORTA_PUR_R   .field 0x40004510,32
GPIO_PORTA_DEN_R   .field 0x4000451C,32
GPIO_PORTA_LOCK_R  .field 0x40004520,32
GPIO_PORTA_CR_R    .field 0x40004524,32
GPIO_PORTA_AMSEL_R .field 0x40004528,32
GPIO_PORTA_PCTL_R  .field 0x4000452C,32

GPIO_PORTF_DIR_R   .field 0x40025400,32
GPIO_PORTF_AFSEL_R .field 0x40025420,32
GPIO_PORTF_PUR_R   .field 0x40025510,32
GPIO_PORTF_DEN_R   .field 0x4002551C,32
GPIO_PORTF_LOCK_R  .field 0x40025520,32
GPIO_PORTF_CR_R    .field 0x40025524,32
GPIO_PORTF_AMSEL_R .field 0x40025528,32
GPIO_PORTF_PCTL_R  .field 0x4002552C,32
GPIO_LOCK_KEY      .field 0x4C4F434B,32  ; Unlocks the GPIO_CR register
PF0                .field 0x40025004,32
PF4                .field 0x40025040,32
PA5                .field 0x40004080,32
SWITCHES           .field 0x40025044,32
DELAY10MS   .field 160000,32    ; clock cycles in 10 ms (assumes 16 MHz clock)
DELAY10US   .field 160,32       ; clock cycles in 10 os (assumes 16 MHz clock)
N1000       .field 1000,32
SW1                .equ 0x10                 ; on the left side of the Launchpad board
SW2                .equ 0x01                 ; on the right side of the Launchpad board
SYSCTL_RCGCGPIO_R  .field 0x400FE608,32
SYSCTL_RCGC2_GPIOF .equ 0x00000020  ; port F Clock Gating Control


        .global  Switch_Init
        .global  Switch_Input
        .global  Board_Init
        .global  Board_Input
        .global   SysTick_Init
        .global   SysTick_Wait
        .global   SysTick_Wait10ms

;------------Switch_Init------------
; Initialize GPIO Port A bit 5 as input
; Input: none
; Output: none
; Modifies: R0, R1
Switch_Init:  .asmfunc
    LDR R1, SYSCTL_RCGCGPIO_R         ; 1) activate clock for Port A
    LDR R0, [R1]
    ORR R0, R0, #0x01               ; set bit 0 to turn on clock
    STR R0, [R1]
    NOP
    NOP                             ; allow time for clock to finish
                                    ; 2) no need to unlock Port A
    LDR R1, GPIO_PORTA_AMSEL_R     ; 3) disable analog functionality
    LDR R0, [R1]
    BIC R0, #0x20                   ; 0 means analog is off
    STR R0, [R1]
    LDR R1, GPIO_PORTA_PCTL_R      ; 4) configure as GPIO
    LDR R0, [R1]
    BIC R0, #0x00F00000             ; 0 means configure PA5 as GPIO
    STR R0, [R1]
    LDR R1, GPIO_PORTA_DIR_R       ; 5) set direction register
    LDR R0, [R1]
    BIC R0, #0x20                   ; PA5 input
    STR R0, [R1]
    LDR R1, GPIO_PORTA_AFSEL_R     ; 6) regular port function
    LDR R0, [R1]
    BIC R0, #0x20                   ; 0 means disable alternate function
    STR R0, [R1]
    LDR R1, GPIO_PORTA_DEN_R       ; 7) enable Port A digital port
    LDR R0, [R1]
    ORR R0, #0x20                   ; 1 means enable digital I/O
    STR R0, [R1]
    BX  LR
    .endasmfunc

;------------Switch_Input------------
; Read and return the status of PA5
; Input: none
; Output: R0  0x20 if PA5 high
;         R0  0x00 if PA5 low
; Modifies: R1
Switch_Input:  .asmfunc
    LDR R1, PA5       ; pointer to PA5
    LDR R0, [R1]      ; read just PA5
    BX  LR            ; 0x20 or 0x00
    .endasmfunc

;------------Board_Init------------
; Initialize GPIO Port F for negative logic switches on PF0 and
; PF4 as the Launchpad is wired.  Weak internal pull-up
; resistors are enabled, and the NMI functionality on PF0 is
; disabled.
; Input: none
; Output: none
; Modifies: R0, R1
Board_Init:  .asmfunc
    ; activate clock for Port F
    LDR R1, SYSCTL_RCGCGPIO_R       ; R1 = &SYSCTL_RCGCGPIO_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #SYSCTL_RCGC2_GPIOF ; R0 = R0|SYSCTL_RCGC2_GPIOF
    STR R0, [R1]                    ; [R1] = R0
    NOP
    NOP                             ; allow time to finish activating
    ; unlock the lock register
    LDR R1, GPIO_PORTF_LOCK_R       ; R1 = &GPIO_PORTF_LOCK_R
    LDR R0, GPIO_LOCK_KEY           ; R0 = GPIO_LOCK_KEY (unlock GPIO Port F Commit Register)
    STR R0, [R1]                    ; [R1] = R0 = 0x4C4F434B
    ; set commit register
    LDR R1, GPIO_PORTF_CR_R         ; R1 = &GPIO_PORTF_CR_R
    MOV R0, #0xFF                   ; R0 = 0x01 (enable commit for PF0)
    STR R0, [R1]                    ; [R1] = R0 = 0x1
    ; set direction register
    LDR R1, GPIO_PORTF_DIR_R        ; R1 = &GPIO_PORTF_DIR_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #(SW1+SW2)          ; R0 = R0&~(SW1|SW2) (make PF0 and PF4 input; PF0 and PF4 built-in buttons)
    STR R0, [R1]                    ; [R1] = R0
    ; regular port function
    LDR R1, GPIO_PORTF_AFSEL_R      ; R1 = &GPIO_PORTF_AFSEL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #(SW1+SW2)          ; R0 = R0&~(SW1|SW2) (disable alt funct on PF0 and PF4)
    STR R0, [R1]                    ; [R1] = R0
    ; put a delay here if you are seeing erroneous NMI
    ; enable pull-up resistors
    LDR R1, GPIO_PORTF_PUR_R        ; R1 = &GPIO_PORTF_PUR_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #(SW1+SW2)          ; R0 = R0|(SW1|SW2) (enable weak pull-up on PF0 and PF4)
    STR R0, [R1]                    ; [R1] = R0
    ; enable digital port
    LDR R1, GPIO_PORTF_DEN_R        ; R1 = &GPIO_PORTF_DEN_R
    LDR R0, [R1]                    ; R0 = [R1]
    ORR R0, R0, #(SW1+SW2)          ; R0 = R0|(SW1|SW2) (enable digital I/O on PF0 and PF4)
    STR R0, [R1]                    ; [R1] = R0
    ; configure as GPIO
    LDR R1, GPIO_PORTF_PCTL_R       ; R1 = &GPIO_PORTF_PCTL_R
    LDR R0, [R1]                    ; R0 = [R1]
    BIC R0, R0, #0x000F000F         ; R0 = R0&~0x000F000F (clear port control field for PF0 and PF4)
    ADD R0, R0, #0x00000000         ; R0 = R0+0x00000000 (configure PF0 and PF4 as GPIO)
    STR R0, [R1]                    ; [R1] = R0
    ; disable analog functionality
    LDR R1, GPIO_PORTF_AMSEL_R      ; R1 = &GPIO_PORTF_AMSEL_R
    MOV R0, #0                      ; R0 = 0 (disable analog functionality on PF)
    STR R0, [R1]                    ; [R1] = R0
    BX  LR                          ; return
    .endasmfunc

;------------Board_Input------------
; Read and return the status of the switches.
; Input: none
; Output: R0  0x01 if only Switch 1 is pressed
;         R0  0x10 if only Switch 2 is pressed
;         R0  0x00 if both switches are pressed
;         R0  0x11 if no switches are pressed
; Modifies: R1
Board_Input:  .asmfunc
    LDR R1, SWITCHES                ; R1 = &SWITCHES (pointer to location of PF0 and PF4)
    LDR R0, [R1]                    ; R0 = [R1] (read PF0 and PF4)
    BX  LR                          ; return
    .endasmfunc


;------------Switch_Debounce------------
; Read and return the status of PA5
; Input: none
; Output: R0  0x20 if PA5 high
;         R0  0x00 if PA5 low
; Modifies: R1
; returns a stable value
Switch_Debounce:  .asmfunc
    PUSH {R4,R5,R6,LR}
    BL   Switch_Input ; Prog 4.2
    MOV  R5,R0        ; switch value
loop1
    LDR  R4, N1000  ; 10ms = 1000*10us
loop2
    LDR  R0, DELAY10US   ;assume 16 MHz
    BL   SysTick_Wait ; wait 10us
    BL   Switch_Input ; Prog 4.2
    CMP  R0,R5
    BNE  loop1 ; start over if different
    SUBS R4,#1 ; same for 10ms?
    BNE  loop2
    POP  {R4,R5,R6,PC}
    .endasmfunc

;------------Switch_Debounce------------
; wait for the switch to be touched
; Input: none
; Output: none
; Modifies: R1
Switch_WaitForTouch:  .asmfunc
    PUSH {R4,LR}
wait4Release
    BL   Switch_Input ; Prog 4.2
    CMP  R0,#0
    BNE  wait4Release ;already touched
    LDR  R0,DELAY10MS   ;assume 16 MHz
    BL   SysTick_Wait ; wait 10ms
wait4Touch
    BL   Switch_Input ; Prog 4.2
    CMP  R0,#0
    BEQ  wait4Touch   ;wait for touch
    LDR  R0,DELAY10MS   ; 10ms
    BL   SysTick_Wait ; wait 10ms
    POP  {R4,PC}
    .endasmfunc

     .end                             ; end of file
