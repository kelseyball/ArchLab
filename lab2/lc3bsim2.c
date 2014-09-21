/*
    Name 1: Jianyu Huang
    UTEID 1: jh57266
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);

    CURRENT_LATCHES.REGS[1] = 4;
	CURRENT_LATCHES.REGS[5] = 0xffff;
	CURRENT_LATCHES.REGS[6] = 1;


}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

void setcc(int res) {
  int sign;
  if (!res) {
	CURRENT_LATCHES.N = 0;
	CURRENT_LATCHES.Z = 1;
	CURRENT_LATCHES.P = 0;
  } else {
	sign = (res >> 15) & 0x1;
	if (!sign) {
	  CURRENT_LATCHES.N = 0;
	  CURRENT_LATCHES.Z = 0;
	  CURRENT_LATCHES.P = 1;
	} else {
	  CURRENT_LATCHES.N = 1;
	  CURRENT_LATCHES.Z = 0;
	  CURRENT_LATCHES.P = 0;
	}
  }
}


int sext(int num, int digits) {
  int sign = (num >> (digits - 1)) & 0x1;
  if (!sign) {
	return num;
  } else {
	return num - (1 << digits);
  }
}

/*
int sext(int sign, int num, int digits) {
  if (sign) {
	return num;
  } else {
	num |= ((-1) << (digits - 1));
	return num;
  }
}
*/

#define OPCODE(x) 
#define DR(x) (((x) >> 9) & 0x7) /* 0b111 */
#define SR1(x) (((x) >> 6) & 0x7) /* 0b111 */


void execADD(instr) {
  int dr, sr1, sr2, steer5, imm5;
  dr = (instr >> 9) & 0x7; /* 0b111 */
  sr1 = (instr >> 6) & 0x7; /* 0b111 */
  steer5 = (instr >> 5) & 0x1; /* 0b1 */
  if (!steer5) {
	sr2 = instr & 0x7; /* 0b111 */
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + CURRENT_LATCHES.REGS[sr2]);
  } else {
	imm5 = instr & 0x1F; /* 0b11111 */
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] + sext(imm5, 5));
  }
  setcc(CURRENT_LATCHES.REGS[dr]);
}

void execAND(instr) {
  int dr, sr1, sr2, steer5, imm5;
  dr = (instr >> 9) & 0x7; /* 0b111 */
  sr1 = (instr >> 6) & 0x7; /* 0b111 */
  steer5 = (instr >> 5) & 0x1; /* 0b1 */
  if (!steer5) {
	sr2 = instr & 0x7; /* 0b111 */
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & CURRENT_LATCHES.REGS[sr2]);
  } else {
	imm5 = instr & 0x1F; /* 0b11111 */
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] & sext(imm5, 5));
  }
  setcc(CURRENT_LATCHES.REGS[dr]);
}

void execXOR(instr) {
  int dr, sr1, sr2, steer5, imm5;
  dr = (instr >> 9) & 0x7; /* 0b111 */
  sr1 = (instr >> 6) & 0x7; /* 0b111 */
  steer5 = (instr >> 5) & 0x1; /* 0b1 */
  if (!steer5) {
	sr2 = instr & 0x7; /* 0b111 */
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ CURRENT_LATCHES.REGS[sr2]);
  } else {
	imm5 = instr & 0x1F; /* 0b11111 */
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] ^ sext(imm5, 5));
  }
  setcc(CURRENT_LATCHES.REGS[dr]);
}

void execSHF(instr) {
  int dr, sr1, steer45, amount4;
  dr = (instr >> 9) & 0x7; /* 0b111 */
  sr1 = (instr >> 6) & 0x7; /* 0b111 */
  amount4 = instr & 0xF; /* 0b1111 */
  steer45 = (instr >> 4) & 0x3; /* 0b11 */
  if (!steer45) {
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] << sext(amount4, 4));
  } else if (steer45 == 1) {
	CURRENT_LATCHES.REGS[dr] = Low16bits((unsigned int)CURRENT_LATCHES.REGS[sr1] >> sext(amount4, 4));
  } else {
	CURRENT_LATCHES.REGS[dr] = Low16bits(CURRENT_LATCHES.REGS[sr1] >> sext(amount4, 4));
  }
  setcc(CURRENT_LATCHES.REGS[dr]);
}

void execBR(instr) {
  int n, z, p, pcoffset9;
  n = (instr >> 11) & 0x1;
  z = (instr >> 10) & 0x1;
  p = (instr >> 9) & 0x1;
  pcoffset9 = instr & 0x1FF;
  if ((n & CURRENT_LATCHES.N) || (z & CURRENT_LATCHES.Z) || (p & CURRENT_LATCHES.P)) {
	CURRENT_LATCHES.PC += (sext(pcoffset9, 9) << 1); /* What if BR to somewhere out of border? Impossible...Because assembler get the address by label, the label address should exist... -> start: 0xFFFF, then label: ****?, but load_program will handle this case*/
  }
}

void execJMP(instr) {

}

void execJSR(instr) {
}

void execLDB(instr) {

}

void execLDW(instr) {
}


void execLDW(instr) {
}

void execLEA(instr) {
}

void execSTB(instr) {
}

void execSTW(instr) {
}

void execTRAP(instr) {
}
	

void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     

    int pc, opcode, instr = 0;
	pc = CURRENT_LATCHES.PC;
	instr = 0;
	instr |= MEMORY[pc >> 1][0];
	instr |= MEMORY[pc >> 1][1] << 8;

	printf("pc: 0x%x\n", pc);
	printf("instr: %x\n", instr);
	printf("0x%x\n", MEMORY[pc/2][1]);
	printf("0x%x\n", MEMORY[pc/2][0]);


	opcode = (instr & 0xF000) >> 12;

	printf("opcode:0x%x\n", opcode);

	CURRENT_LATCHES.PC += 2;
	/*
    CURRENT_LATCHES = NEXT_LATCHES;
	Is this a copy-by-address or copy-by-value?
	*/

	switch (opcode) {
	  case 0x1: /* ADD */
		execADD(instr);
		break;
	  case 0x5: /* AND */
		execAND(instr);
		break;
	  case 0x9: /* XOR/NOT */
		execXOR(instr);
		break;
	  case 0xD: /* LSHF, RSHFL, RSHFA */
		execSHF(instr);
		break;
	  case 0x0: /* BR */
		execBR(instr);
		break;
	  case 0xC: /* JMP, RET */
		execJMP(instr);
		break;
	  case 0x4:	 /* JSR, JSRR */ 
		execJSR(instr);
		break;
	  case 0x2: /* LDB */
		execLDB(instr);
		break;
	  case 0x6: /* LDW */
		execLDW(instr);
		break;
	  case 0xE: /* LEA */
		execLEA(instr);
		break;
	  case 0x3: /* STB */
		execSTB(instr);
		break;
	  case 0x7: /* STW */
		execSTW(instr);
		break;
	  case 0xF: /* TRAP */
		execTRAP(instr);
		break;
	  case 0x8: /* RTI */
		execRTI(instr); /* NOT IMPLEMENTED */
		break;

	  default:
		//error?
		printf("opcode is 0x10 or 0x11, not used opcode!*");
		break;
	}

    NEXT_LATCHES = CURRENT_LATCHES;
}







