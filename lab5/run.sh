gcc -ansi assembler.c -o assemble
./assemble test/$1.asm test/$1.obj
gcc -ansi lc3bsim5.c -o lc3bsim5
./lc3bsim5 ucode5 test/$1.obj data.obj vector_table.obj int.obj except_prot.obj except_unaligned.obj except_unknown.obj
