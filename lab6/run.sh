gcc -ansi assembler.c -o assemble
./assemble $1.asm $1.obj
gcc -ansi lc3bsim6.c -o lc3bsim6
./lc3bsim6 ucode6 $1.obj
