; PLL.s
; Runs on LM4F120/TM4C123
; A software function to change the bus speed using the PLL.
; The EQU statement allows the function PLL_Init() to
; initialize the PLL to the desired frequency.
; Daniel Valvano
; May 5, 2015

;  This example accompanies the book
;  "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
;  ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2013
;  Program 4.6, Section 4.3
;  Volume 2 Program 2.10, Figure 2.37
;
;Copyright 2013 by Jonathan W. Valvano, valvano@mail.utexas.edu
;   You may use, edit, run or distribute this file
;   as long as the above copyright notice remains
;THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
;OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
;MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
;VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
;OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;For more information about my classes, my research, and my books, see
;http://users.ece.utexas.edu/~valvano/

; The EQU statement SYSDIV2 initializes
; the PLL to the desired frequency.
SYSDIV2                EQU 4
; bus frequency is 400MHz/(SYSDIV2+1) = 400MHz/(4+1) = 80 MHz

SYSCTL_RIS_R           EQU 0x400FE050
SYSCTL_RIS_PLLLRIS     EQU 0x00000040  ; PLL Lock Raw Interrupt Status
SYSCTL_RCC_R           EQU 0x400FE060
SYSCTL_RCC_XTAL_M      EQU 0x000003C0  ; Crystal Value
SYSCTL_RCC_XTAL_6MHZ   EQU 0x000002C0  ; 6 MHz Crystal
SYSCTL_RCC_XTAL_8MHZ   EQU 0x00000380  ; 8 MHz Crystal
SYSCTL_RCC_XTAL_16MHZ  EQU 0x00000540  ; 16 MHz Crystal
SYSCTL_RCC2_R          EQU 0x400FE070
SYSCTL_RCC2_USERCC2    EQU 0x80000000  ; Use RCC2
SYSCTL_RCC2_DIV400     EQU 0x40000000  ; Divide PLL as 400 MHz vs. 200
                                       ; MHz
SYSCTL_RCC2_SYSDIV2_M  EQU 0x1F800000  ; System Clock Divisor 2
SYSCTL_RCC2_SYSDIV2LSB EQU 0x00400000  ; Additional LSB for SYSDIV2
SYSCTL_RCC2_PWRDN2     EQU 0x00002000  ; Power-Down PLL 2
SYSCTL_RCC2_BYPASS2    EQU 0x00000800  ; PLL Bypass 2
SYSCTL_RCC2_OSCSRC2_M  EQU 0x00000070  ; Oscillator Source 2
SYSCTL_RCC2_OSCSRC2_MO EQU 0x00000000  ; MOSC

        AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        EXPORT  PLL_Init

;------------PLL_Init------------
; Configure the system to get its clock from the PLL.
; Input: none
; Output: none
; Modifies: R0, R1, R2, R3
PLL_Init
    ; 0) configure the system to use RCC2 for advanced features
    ;    such as 400 MHz PLL and non-integer System Clock Divisor
    LDR R3, =SYSCTL_RCC2_R          ; R3 = SYSCTL_RCC2_R (pointer)
    LDR R2, [R3]                    ; R2 = [R3] (value)
    ORR R2, R2, #SYSCTL_RCC2_USERCC2; R2 = R2|SYSCTL_RCC2_USERCC2
    STR R2, [R3]                    ; [R3] = R2
    ; 1) bypass PLL while initializing
    ORR R2, R2, #SYSCTL_RCC2_BYPASS2; R2 = R2|SYSCTL_RCC2_BYPASS2
    STR R2, [R3]                    ; [R3] = R2
    ; 2) select the crystal value and oscillator source
    LDR R1, =SYSCTL_RCC_R           ; R1 = SYSCTL_RCC_R (pointer)
    LDR R0, [R1]                    ; R0 = [R1] (value)
    BIC R0, R0, #SYSCTL_RCC_XTAL_M  ; R0 = R0&~SYSCTL_RCC_XTAL_M (clear XTAL field)
    ORR R0, R0, #SYSCTL_RCC_XTAL_16MHZ;R0 = R0|SYSCTL_RCC_XTAL_16MHZ (configure for 16 MHz crystal)
    STR R0, [R1]                    ; [R1] = R0
                                    ; R2 = R2&~SYSCTL_RCC2_OSCSRC2_M (clear oscillator source field)
    BIC R2, R2, #SYSCTL_RCC2_OSCSRC2_M
                                    ; R2 = R2|SYSCTL_RCC2_OSCSRC2_MO (configure for main oscillator source)
    ORR R2, R2, #SYSCTL_RCC2_OSCSRC2_MO
    ; 3) activate PLL by clearing PWRDN
    BIC R2, R2, #SYSCTL_RCC2_PWRDN2 ; R2 = R2&~SYSCTL_RCC2_PWRDN2
    ; 4) set the desired system divider and the system divider least significant bit
    ORR R2, R2, #SYSCTL_RCC2_DIV400 ; R2 = R2|SYSCTL_RCC2_DIV400 (use 400 MHz PLL)
                                    ;R2 = R2&~SYSCTL_RCC2_SYSDIV2_M (clear system clock divider field)
    BIC R2, R2, #SYSCTL_RCC2_SYSDIV2_M
                                    ;R2 = R2&~SYSCTL_RCC2_SYSDIV2LSB (clear bit SYSDIV2LSB)
    BIC R2, R2, #SYSCTL_RCC2_SYSDIV2LSB
    ; set Divisor by writing 7-bit constant to bits 28-22
    ADD R2, R2, #(SYSDIV2<<22)      ; R2 = R2 + (SYSDIV2<<22) (configure SYSDIV2 and SYSDIV2LSB fields in RCC2)
    STR R2, [R3]                    ; [R3] = R2
    ; 5) wait for the PLL to lock by polling PLLLRIS
    LDR R1, =SYSCTL_RIS_R           ; R1 = SYSCTL_RIS_R (pointer)
PLL_Init_loop
    LDR R0, [R1]                    ; R0 = [R1] (value)
    ANDS R0, R0, #SYSCTL_RIS_PLLLRIS; R0 = R0&SYSCTL_RIS_PLLLRIS
    BEQ PLL_Init_loop               ; if(R0 == 0), keep polling
    ; 6) enable use of PLL by clearing BYPASS
    BIC R2, R2, #SYSCTL_RCC2_BYPASS2; R2 = R2&~SYSCTL_RCC2_BYPASS2
    STR R2, [R3]                    ; [R3] = R2
    BX  LR                          ; return

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file

; SYSDIV2  Divisor  Clock (MHz)
; 0        1       reserved
; 1        2       reserved
; 2        3       reserved
; 3        4       reserved
; 4        5       80.000
; 5        6       66.667
; 6        7       reserved
; 7        8       50.000
; 8        9       44.444
; 9        10      40.000
; 10       11      36.364
; 11       12      33.333
; 12       13      30.769
; 13       14      28.571
; 14       15      26.667
; 15       16      25.000
; 16       17      23.529
; 17       18      22.222
; 18       19      21.053
; 19       20      20.000
; 20       21      19.048
; 21       22      18.182
; 22       23      17.391
; 23       24      16.667
; 24       25      16.000
; 25       26      15.385
; 26       27      14.815
; 27       28      14.286
; 28       29      13.793
; 29       30      13.333
; 30       31      12.903
; 31       32      12.500
; 32       33      12.121
; 33       34      11.765
; 34       35      11.429
; 35       36      11.111
; 36       37      10.811
; 37       38      10.526
; 38       39      10.256
; 39       40      10.000
; 40       41      9.756
; 41       42      9.524
; 42       43      9.302
; 43       44      9.091
; 44       45      8.889
; 45       46      8.696
; 46       47      8.511
; 47       48      8.333
; 48       49      8.163
; 49       50      8.000
; 50       51      7.843
; 51       52      7.692
; 52       53      7.547
; 53       54      7.407
; 54       55      7.273
; 55       56      7.143
; 56       57      7.018
; 57       58      6.897
; 58       59      6.780
; 59       60      6.667
; 60       61      6.557
; 61       62      6.452
; 62       63      6.349
; 63       64      6.250
; 64       65      6.154
; 65       66      6.061
; 66       67      5.970
; 67       68      5.882
; 68       69      5.797
; 69       70      5.714
; 70       71      5.634
; 71       72      5.556
; 72       73      5.479
; 73       74      5.405
; 74       75      5.333
; 75       76      5.263
; 76       77      5.195
; 77       78      5.128
; 78       79      5.063
; 79       80      5.000
; 80       81      4.938
; 81       82      4.878
; 82       83      4.819
; 83       84      4.762
; 84       85      4.706
; 85       86      4.651
; 86       87      4.598
; 87       88      4.545
; 88       89      4.494
; 89       90      4.444
; 90       91      4.396
; 91       92      4.348
; 92       93      4.301
; 93       94      4.255
; 94       95      4.211
; 95       96      4.167
; 96       97      4.124
; 97       98      4.082
; 98       99      4.040
; 99       100     4.000
; 100      101     3.960
; 101      102     3.922
; 102      103     3.883
; 103      104     3.846
; 104      105     3.810
; 105      106     3.774
; 106      107     3.738
; 107      108     3.704
; 108      109     3.670
; 109      110     3.636
; 110      111     3.604
; 111      112     3.571
; 112      113     3.540
; 113      114     3.509
; 114      115     3.478
; 115      116     3.448
; 116      117     3.419
; 117      118     3.390
; 118      119     3.361
; 119      120     3.333
; 120      121     3.306
; 121      122     3.279
; 122      123     3.252
; 123      124     3.226
; 124      125     3.200
; 125      126     3.175
; 126      127     3.150
; 127      128     3.125
