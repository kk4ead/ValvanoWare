; Timer0A.s
; Runs on LM4F120/TM4C123
; Use Timer0A in periodic mode to request interrupts at a particular
; period.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;   Volume 1, Program 9.8

;  "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
;   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
;   Volume 2, Program 7.5, example 7.6

; Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
;   You may use, edit, run or distribute this file
;   as long as the above copyright notice remains
;THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
;OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
;MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
;VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
;OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;For more information about my classes, my research, and my books, see
;http://users.ece.utexas.edu/~valvano/

NVIC_EN0_INT19        EQU 0x00080000  ; Interrupt 19 enable
NVIC_EN0_R            EQU 0xE000E100  ; IRQ 0 to 31 Set Enable Register
NVIC_PRI4_R           EQU 0xE000E410  ; IRQ 16 to 19 Priority Register
TIMER0_CFG_R          EQU 0x40030000
TIMER_CFG_32_BIT      EQU 0x00000000  ; 32-bit timer configuration,
                                      ; function is controlled by bits
                                      ; 1:0 of GPTMTAMR and GPTMTBMR
TIMER0_TAMR_R         EQU 0x40030004
TIMER_TAMR_TAMR_PERIOD EQU 0x00000002 ; Periodic Timer mode
TIMER0_CTL_R          EQU 0x4003000C
TIMER_CTL_TAEN        EQU 0x00000001  ; GPTM TimerA Enable
TIMER0_IMR_R          EQU 0x40030018
TIMER_IMR_TATOIM      EQU 0x00000001  ; GPTM TimerA Time-Out Interrupt
                                      ; Mask
TIMER0_ICR_R          EQU 0x40030024
TIMER_ICR_TATOCINT    EQU 0x00000001  ; GPTM TimerA Time-Out Raw
                                      ; Interrupt
TIMER0_TAILR_R        EQU 0x40030028
TIMER_TAILR_TAILRL_M  EQU 0x0000FFFF  ; GPTM TimerA Interval Load
                                      ; Register Low
TIMER0_TAPR_R         EQU 0x40030038
SYSCTL_RCGCTIMER_R    EQU   0x400FE604

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT   Timer0A_Init

; ***************** Timer0A_Init ****************
; Activate Timer0A interrupts to run user task periodically
; Input: R0  interrupt period
;        Units of period are 12.5ns (assuming 80 MHz clock)
;        Maximum is 2^16-1
;        Minimum is determined by length of ISR
; Output: none
; Modifies: R0, R1, R2
Timer0A_Init
    ; 0) activate clock for Timer0
    LDR R1, =SYSCTL_RCGCTIMER_R     ; activate Timer0 clock
    LDR R2, [R1]                    
    ORR R2, R2, #0x01       
    STR R2, [R1]                  
    NOP
    NOP                             ; allow time to finish activating
    ; 1) disable timer0A during setup
    LDR R1, =TIMER0_CTL_R           
    LDR R2, [R1]                   
    BIC R2, R2, #0x00000001         ; clear enable bit
    STR R2, [R1]                   
    ; 2) configure for 32-bit timer mode
    LDR R1, =TIMER0_CFG_R          
    MOV R2, #0x00000000             ; 32-bit mode
    STR R2, [R1]                
    ; 3) configure for periodic mode
    LDR R1, =TIMER0_TAMR_R         
    MOV R2, #0x00000002             ; periodic mode
    STR R2, [R1]                   
    ; 4) reload value, period =(TAILR+1)*12.5ns
    LDR R1, =TIMER0_TAILR_R        
    SUB R0, R0, #1                  ; counts down from TAILR to 0
    STR R0, [R1]                  
    ; 5) 1us timer0A
    LDR R1, =TIMER0_TAPR_R          
    MOV R2, #0                      ; R2 = 0 (divide clock by 1)
    STR R2, [R1]                   
    ; 6) clear timer0A timeout flag
    LDR R1, =TIMER0_ICR_R          
    MOV R2, #0x00000001             ; clear TATORIS bit
    STR R2, [R1]                 
    ; 7) arm timeout interrupt
    LDR R1, =TIMER0_IMR_R           
    MOV R2, #0x00000001             ; Arm TATORIS
    STR R2, [R1]                    
    ; 8) set NVIC interrupt 19 to priority 2
    LDR R1, =NVIC_PRI4_R           
    LDR R2, [R1]                    
    AND R2, R2, #0x00FFFFFF         ; clear interrupt 19 priority
    ORR R2, R2, #0x40000000         ; interrupt 19 priority is in bits 31-29
    STR R2, [R1]                  
    ; 9) enable interrupt 19 in NVIC
    LDR R1, =NVIC_EN0_R            
    MOV R2, #0x00080000             ; interrupt 19 enabled
    STR R2, [R1]                   
    ; 10) enable timer0A
    LDR R1, =TIMER0_CTL_R         
    LDR R2, [R1]                  
    ORR R2, R2, #0x00000001         ; set Timer0 enable bit
    STR R2, [R1]                   
    BX  LR                       

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
