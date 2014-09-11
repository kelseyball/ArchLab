;This assembly language program converts a null-terminted character string assembled for an LC-3 to a null-terminated character string assembled for an LC-3b.
        .ORIG x3000
        LEA R0, TOLOAD
        LDW R0, R0, #0  ;x4000 contains the starting address for string to load

        LEA R1, TOSTORE
        LDW R1, R1, #0  ;x4002 contains the starting address for string to store

START   LDB R2, R0, #0
        BRZ DONE
        STB R2, R1, #0
        ADD R0, R0, #2
        ADD R1, R1, #1
        BR  START

DONE    TRAP x25
TOLOAD  .FILL x4000
TOSTORE .FILL x4002
.END
