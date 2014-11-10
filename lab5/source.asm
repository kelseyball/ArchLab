;This program counts from 10 to 0
        .ORIG x3000
;        ADD R1, R6, R5
;        AND R2, R3, R4
;        NOT R2, R3
;        XOR R2, R3, R4
;        LSHF R1, R2, #3
;        RSHFL R0, R1, #3
;        RSHFA R0, R1, #3
;        JSR TEST
;TEST    NOP
;        RET

        JSRR R5
        NOP
        JMP R7



        .END

