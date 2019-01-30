;/*****************************************************************************/
;/* OSasm.s: low-level OS commands, written in assembly                       */
;/* derived from uCOS-II                                                      */
;/*****************************************************************************/
;Jonathan Valvano, Fixed Scheduler, 5/3/2015


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8

        EXTERN  RunPt            ; currently running thread
        EXTERN  NodePt           ; linked list of treads to run
        EXTERN  sys
        EXPORT  OS_Launch
        EXPORT  SysTick_Handler
        EXPORT  PendSV_Handler


NVIC_INT_CTRL   EQU     0xE000ED04                              ; Interrupt control state register.
NVIC_SYSPRI14   EQU     0xE000ED22                              ; PendSV priority register (position 14).
NVIC_SYSPRI15   EQU     0xE000ED23                              ; Systick priority register (position 15).
NVIC_LEVEL14    EQU           0xEF                              ; Systick priority value (second lowest).
NVIC_LEVEL15    EQU           0xFF                              ; PendSV priority value (lowest).
NVIC_PENDSVSET  EQU     0x10000000                              ; Value to trigger PendSV exception.
NVIC_ST_RELOAD  EQU     0xE000E014
NVIC_ST_CURRENT EQU     0xE000E018




OS_Launch
    LDR     R0, =RunPt         ; currently running thread
    LDR     R2, [R0]		   ; R2 = value of RunPt
    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11 
    POP     {R0-R3}            ; restore regs r0-3 
    POP     {R12}
    POP     {LR}               ; discard LR from initial stack
    POP     {LR}               ; start location
    POP     {R1}               ; discard PSR
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread

OSStartHang
    B       OSStartHang        ; Should never get here



;********************************************************************************************************

SysTick_Handler
    CPSID   I                  ; Prevent interruption during context switch
    PUSH    {R4-R11}           ; Save remaining regs r4-11 
    LDR     R0, =RunPt         ; R0=pointer to RunPt, old thread
    LDR     R1, [R0]		   ; RunPt->stackPointer = SP;
    STR     SP, [R1]           ; save SP of process being switched out
    LDR     R1, =NodePt
    LDR     R2, [R1]           ; NodePt
    LDR     R2, [R2]           ; next to run
    STR     R2, [R1]           ; NodePt= NodePt->Next;
    LDR     R3, [R2,#4]        ; RunPt = &sys[NodePt->Thread];// which thread
    STR     R3, [R0]
   
    LDR     R2, [R2,#8]		   ; NodePt->TimeSlice
    SUB     R2, #50            ; subtract off time to run this ISR
    LDR     R1, =NVIC_ST_RELOAD
    STR     R2, [R1]           ; how long to run next thread
    LDR     R1, =NVIC_ST_CURRENT
    STR     R2, [R1]           ; write to current, clears it

    LDR     SP, [R3]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11 

    CPSIE   I				   ; tasks run with I=0
    BX      LR                 ; Exception return will restore remaining context   
    
PendSV_Handler
    CPSID   I                  ; Prevent interruption during context switch
    PUSH    {R4-R11}           ; Save remaining regs r4-11 
    LDR     R0, =RunPt         ; R0=pointer to RunPt, old thread
    LDR     R1, [R0]		   ; RunPt->stackPointer = SP;
    STR     SP, [R1]           ; save SP of process being switched out
    LDR     R1, =sys           ; each sys[] is 400 bytes
    ADD     R1,#3*400          ; pointer to sys[3] = ThePan (background task)
    STR     R1, [R0]           ; RunPt = ThePan
    LDR     SP, [R1]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11 

    CPSIE   I				   ; tasks run with I=0
    BX      LR                 ; Exception return will restore remaining context   

    ALIGN
    END
