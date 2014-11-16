        .ORIG x3000
        LEA R0, USPTR
        LDW R6, R0, #0

        XOR R0, R0, R0
        ADD R0, R0, #1
        LEA R1, MOVLOC
        LDW R1, R1, #0
        STB R0, R1, #0

        HALT

USPTR   .FILL xFE00
MOVLOC  .FILL xC050

;ADDLOC  .FILL xC000
;SUMLOC  .FILL xC014
;SUMLOC  .FILL xC017
SUMLOC  .FILL x0000
        .END
