; main.asm
; Runs on LM4F120/TM4C123
; Test the GPIO initialization functions by setting the LED
; color according to the status of the switches.
; The Reflex Test (no longer supported; each LED turns others off):
; This program is functionally similar to SwitchTestMain.c
; in Switch_4F120asm.  When switch #1 is pressed, the blue
; LED comes on.  When switch #2 is pressed, the red LED
; comes on.  When both switches are pressed, the green LED
; comes on.  A short delay is inserted between
; polls of the buttons to compensate for your reflexes and
; the button bounce.  The following color combinations can
; be made:
; Color    LED(s) Illumination Method
; dark     ---    release both buttons
; red      R--    press right button (#2)
; blue     --B    press left button (#1)
; green    -G-    press both buttons exactly together
; yellow   RG-    press right button, then press left button
; sky blue -GB    press left button, then press right button
; white    RGB    press either button, then press the other
;                 button, then release the first button
; pink     R-B    press either button, then release the
;                 first button and immediately press the
;                 other button
; Daniel Valvano
; May 3, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to ARM Cortex M Microcontrollers"
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
;  Section 4.2    Program 4.1
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

; negative logic switch #2 connected to PF0 on the Launchpad
; red LED connected to PF1 on the Launchpad
; blue LED connected to PF2 on the Launchpad
; green LED connected to PF3 on the Launchpad
; negative logic switch #1 connected to PF4 on the Launchpad
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
       .align  2
GPIO_PORTF_DATA_R  .field 0x400253FC,32
GPIO_PORTF_DIR_R   .field 0x40025400,32
GPIO_PORTF_AFSEL_R .field 0x40025420,32
GPIO_PORTF_PUR_R   .field 0x40025510,32
GPIO_PORTF_DEN_R   .field 0x4002551C,32
GPIO_PORTF_LOCK_R  .field 0x40025520,32
GPIO_PORTF_CR_R    .field 0x40025524,32
GPIO_PORTF_AMSEL_R .field 0x40025528,32
GPIO_PORTF_PCTL_R  .field 0x4002552C,32
GPIO_LOCK_KEY      .field 0x4C4F434B,32  ; Unlocks the GPIO_CR register
RED       .equ 0x02
BLUE      .equ 0x04
GREEN     .equ 0x08
SW1       .equ 0x10                 ; on the left side of the Launchpad board
SW2       .equ 0x01                 ; on the right side of the Launchpad board
SYSCTL_RCGCGPIO_R  .field   0x400FE608,32

      .global main

main:  .asmfunc
    BL  PortF_Init                  ; initialize input and output pins of Port F
loop
    LDR R0, FIFTHSEC                ; R0 = FIFTHSEC (delay 0.2 second)
    BL  delay                       ; delay at least (3*R0) cycles
    BL  PortF_Input                 ; read all of the switches on Port F
    CMP R0, #0x01                   ; R0 == 0x01?
    BEQ sw1pressed                  ; if so, switch 1 pressed
    CMP R0, #0x10                   ; R0 == 0x10?
    BEQ sw2pressed                  ; if so, switch 2 pressed
    CMP R0, #0x00                   ; R0 == 0x00?
    BEQ bothpressed                 ; if so, both switches pressed
    CMP R0, #0x11                   ; R0 == 0x11?
    BEQ nopressed                   ; if so, neither switch pressed
                                    ; if none of the above, unexpected return value
    MOV R0, #(RED+GREEN+BLUE)       ; R0 = (RED|GREEN|BLUE) (all LEDs on)
    BL  PortF_Output                ; turn all of the LEDs on
    B   loop
sw1pressed
    MOV R0, #BLUE                   ; R0 = BLUE (blue LED on)
    BL  PortF_Output                ; turn the blue LED on
    B   loop
sw2pressed
    MOV R0, #RED                    ; R0 = RED (red LED on)
    BL  PortF_Output                ; turn the red LED on
    B   loop
bothpressed
    MOV R0, #GREEN                  ; R0 = GREEN (green LED on)
    BL  PortF_Output                ; turn the green LED on
    B   loop
nopressed
    MOV R0, #0                      ; R0 = 0 (no LEDs on)
    BL  PortF_Output                ; turn all of the LEDs off
    B   loop
    .endasmfunc
;------------delay------------
; Delay function for testing, which delays about 3*count cycles.
; Input: R0  count
; Output: none
ONESEC             .field 5333333,32      ; approximately 1s delay at ~16 MHz clock
QUARTERSEC         .field 1333333,32      ; approximately 0.25s delay at ~16 MHz clock
FIFTHSEC           .field 1066666,32      ; approximately 0.2s delay at ~16 MHz clock
delay:  .asmfunc
    SUBS R0, R0, #1                 ; R0 = R0 - 1 (count = count - 1)
    BNE delay                       ; if count (R0) != 0, skip to 'delay'
    BX  LR                          ; return
    .endasmfunc

;------------PortF_Init------------
; Initialize GPIO Port F for negative logic switches on PF0 and
; PF4 as the Launchpad is wired.  Weak internal pull-up
; resistors are enabled, and the NMI functionality on PF0 is
; disabled.  Make the RGB LED's pins outputs.
; Input: none
; Output: none
; Modifies: R0, R1, R2
PortF_Init:  .asmfunc
    LDR R1, SYSCTL_RCGCGPIO_R       ; 1) activate clock for Port F
    LDR R0, [R1]
    ORR R0, R0, #0x20               ; set bit 5 to turn on clock
    STR R0, [R1]
    NOP
    NOP                             ; allow time for clock to finish
    LDR R1, GPIO_PORTF_LOCK_R       ; 2) unlock the lock register
    LDR R0, GPIO_LOCK_KEY             ; unlock GPIO Port F Commit Register
    STR R0, [R1]
    LDR R1, GPIO_PORTF_CR_R         ; enable commit for Port F
    MOV R0, #0xFF                   ; 1 means allow access
    STR R0, [R1]
    LDR R1, GPIO_PORTF_AMSEL_R      ; 3) disable analog functionality
    MOV R0, #0                      ; 0 means analog is off
    STR R0, [R1]
    LDR R1, GPIO_PORTF_PCTL_R       ; 4) configure as GPIO
    MOV R0, #0x00000000             ; 0 means configure Port F as GPIO
    STR R0, [R1]
    LDR R1, GPIO_PORTF_DIR_R        ; 5) set direction register
    MOV R0,#0x0E                    ; PF0 and PF7-4 input, PF3-1 output
    STR R0, [R1]
    LDR R1, GPIO_PORTF_AFSEL_R      ; 6) regular port function
    MOV R0, #0                      ; 0 means disable alternate function
    STR R0, [R1]
    LDR R1, GPIO_PORTF_PUR_R        ; pull-up resistors for PF4,PF0
    MOV R0, #0x11                   ; enable weak pull-up on PF0 and PF4
    STR R0, [R1]
    LDR R1, GPIO_PORTF_DEN_R        ; 7) enable Port F digital port
    MOV R0, #0xFF                   ; 1 means enable digital I/O
    STR R0, [R1]
    BX  LR
    .endasmfunc


;------------PortF_Input------------
; Read and return the status of the switches.
; Input: none
; Output: R0  0x01 if only Switch 1 is pressed
;         R0  0x10 if only Switch 2 is pressed
;         R0  0x00 if both switches are pressed
;         R0  0x11 if no switches are pressed
; Modifies: R1
PortF_Input:  .asmfunc
    LDR R1, GPIO_PORTF_DATA_R  ; pointer to Port F data
    LDR R0, [R1]               ; read all of Port F
    AND R0,R0,#0x11            ; just the input pins PF0 and PF4
    BX  LR                     ; return R0 with inputs
    .endasmfunc

;------------PortF_Output------------
; Set the output state of PF3-1.
; Input: R0  new state of PF
; Output: none
; Modifies: R1
PortF_Output:  .asmfunc
    LDR R1, GPIO_PORTF_DATA_R  ; pointer to Port F data
    STR R0, [R1]               ; write to PF3-1
    BX  LR
    .endasmfunc

    .end                             ; end of file
