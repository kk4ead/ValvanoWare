;/*****************************************************************************/
;/* OSasm.s: low-level OS commands, written in assembly                       */
;/* derived from uCOS-II                                                      */
;/*****************************************************************************/
;Jonathan Valvano, Fixed Scheduler, 5/3/2015

       .thumb

       .text
       .align  2

        .global  RunPt            ; currently running thread
        .global  NodePt           ; linked list of treads to run
        .global  sys
        .global  OS_Launch
        .global  SysTick_Handler
        .global  PendSV_Handler


NVIC_INT_CTRL   .field     0xE000ED04,32             ; Interrupt control state register.
NVIC_SYSPRI14   .field     0xE000ED22,32             ; PendSV priority register (position 14).
NVIC_SYSPRI15   .field     0xE000ED23,32             ; Systick priority register (position 15).
NVIC_LEVEL14    .EQU           0xEF                  ; Systick priority value (second lowest).
NVIC_LEVEL15    .EQU           0xFF                  ; PendSV priority value (lowest).
NVIC_PENDSVSET  .field     0x10000000,32             ; Value to trigger PendSV exception.
NVIC_ST_RELOAD  .field     0xE000E014,32
NVIC_ST_CURRENT .field     0xE000E018,32
RunPtAddr       .field     RunPt,32
NodePtAddr      .field     NodePt,32
sysAddr         .field     sys,32


OS_Launch: .asmfunc
    LDR     R0, RunPtAddr      ; currently running thread
    LDR     R2, [R0]           ; R2 = value of RunPt
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

    .endasmfunc

;********************************************************************************************************

SysTick_Handler: .asmfunc
    CPSID   I                  ; Prevent interruption during context switch
    PUSH    {R4-R11}           ; Save remaining regs r4-11
    LDR     R0, RunPtAddr      ; R0=pointer to RunPt, old thread
    LDR     R1, [R0]           ; RunPt->stackPointer = SP;
    STR     SP, [R1]           ; save SP of process being switched out
    LDR     R1, NodePtAddr
    LDR     R2, [R1]           ; NodePt
    LDR     R2, [R2]           ; next to run
    STR     R2, [R1]           ; NodePt= NodePt->Next;
    LDR     R3, [R2,#4]        ; RunPt = &sys[NodePt->Thread];// which thread
    STR     R3, [R0]

    LDR     R2, [R2,#8]      ; NodePt->TimeSlice
    SUB     R2, #50            ; subtract off time to run this ISR
    LDR     R1, NVIC_ST_RELOAD
    STR     R2, [R1]           ; how long to run next thread
    LDR     R1, NVIC_ST_CURRENT
    STR     R2, [R1]           ; write to current, clears it

    LDR     SP, [R3]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11

    CPSIE   I          ; tasks run with I=0
    BX      LR                 ; Exception return will restore remaining context
    .endasmfunc
PendSV_Handler: .asmfunc
    CPSID   I                  ; Prevent interruption during context switch
    PUSH    {R4-R11}           ; Save remaining regs r4-11
    LDR     R0,RunPtAddr       ; R0=pointer to RunPt, old thread
    LDR     R1, [R0]           ; RunPt->stackPointer = SP;
    STR     SP, [R1]           ; save SP of process being switched out
    LDR     R1, sysAddr        ; each sys[] is 400 bytes
    ADD     R1,#3*400          ; pointer to sys[3] = ThePan (background task)
    STR     R1, [R0]           ; RunPt = ThePan
    LDR     SP, [R1]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11

    CPSIE   I          ; tasks run with I=0
    BX      LR                 ; Exception return will restore remaining context
    .endasmfunc
    .end
