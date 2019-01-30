; SysTickInts.s
; Runs on LM4F120/TM4C123
; Use the SysTick timer to request interrupts at a particular period.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;   Volume 1, Program 9.7
   
;   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;   Volume 2, Program 5.12, section 5.7
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
NVIC_SYS_PRI3_R       EQU 0xE000ED20  ; Sys. Handlers 12 to 15 Priority

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT   SysTick_Init

; **************SysTick_Init*********************
; Initialize SysTick periodic interrupts, priority 2
; Input: R0  interrupt period
;        Units of period are 1/clockfreq
;        Maximum is 2^24-1
;        Minimum is determined by length of ISR
; Output: none
; Modifies: R0, R1, R2, R3
SysTick_Init
    ; start critical section
    MRS    R3, PRIMASK              ; save old status
    CPSID  I                        ; mask all (except faults)
    ; disable SysTick during setup
    LDR R1, =NVIC_ST_CTRL_R        
    MOV R2, #0
    STR R2, [R1]                    ; disable SysTick
    ; maximum reload value
    LDR R1, =NVIC_ST_RELOAD_R      
    SUB R0, R0, #1                  ; counts down from RELOAD to 0
    STR R0, [R1]                    ; establish interrupt period
    ; any write to current clears it
    LDR R1, =NVIC_ST_CURRENT_R      
    STR R2, [R1]                    ; writing to counter clears it
    ; set NVIC system interrupt 15 to priority 2
    LDR R1, =NVIC_SYS_PRI3_R        
    LDR R2, [R1]                   
    AND R2, R2, #0x00FFFFFF         ; R2 = R2&0x00FFFFFF (clear interrupt 15 priority)
    ORR R2, R2, #0x40000000         ; R2 = R2|0x40000000 (interrupt 15 priority is in bits 31-29)
    STR R2, [R1]                    ; set SysTick to priority 2
    ; enable SysTick with core clock
    LDR R1, =NVIC_ST_CTRL_R        
; ENABLE SysTick (bit 0), INTEN enable interrupts (bit 1), and CLK_SRC (bit 2) is internal
    MOV R2, #(NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_INTEN+NVIC_ST_CTRL_CLK_SRC)
    STR R2, [R1]                    ; store a 7 to NVIC_ST_CTRL_R
    ; end critical section
    MSR    PRIMASK, R3              ; restore old status
    BX  LR                          ; return

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
