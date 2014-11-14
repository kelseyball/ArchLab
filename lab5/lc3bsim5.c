/*
   Name 1: Jianyu Huang
   UTEID 1: jh57266
   */

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
  IRD,
  COND1, COND0,
  J5, J4, J3, J2, J1, J0,
  LD_MAR,
  LD_MDR,
  LD_IR,
  LD_BEN,
  LD_REG,
  LD_CC,
  LD_PC,
  GATE_PC,
  GATE_MDR,
  GATE_ALU,
  GATE_MARMUX,
  GATE_SHF,
  PCMUX1, PCMUX0,
  DRMUX,
  SR1MUX,
  ADDR1MUX,
  ADDR2MUX1, ADDR2MUX0,
  MARMUX,
  ALUK1, ALUK0,
  MIO_EN,
  R_W,
  DATA_SIZE,
  LSHF1,
  /* MODIFY: you have to add all your new control signals */
  COND2,
  LD_R6,
  LD_E,
  LD_SSP,
  LD_USP,
  LD_PSR,
  GATE_PSR,
  GATE_VECTOR,
  PSRMUX1, PSRMUX0,
  OPRMUX,
  R6MUX1, R6MUX0,
  CCMUX,
  I_RESET,
  E_RESET,

  V2P,
  LD_VA,
  LD_J,
  GATE_PTBR,
  GATE_VA,
  CK_PROT_PAGE,
  UPDATE_PTE,
  /* Please refer to my readme file in Canvas for the meaning of these control signals */

 
  CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
	(x[J3] << 3) + (x[J2] << 2) +
	(x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */
int GetCOND2(int *x)         { return(x[COND2]); }
int GetLD_R6(int *x)         { return(x[LD_R6]); }
int GetLD_E(int *x)          { return(x[LD_E]); }
int GetLD_SSP(int *x)        { return(x[LD_SSP]); }
int GetLD_USP(int *x)        { return(x[LD_USP]); }
int GetLD_PSR(int *x)        { return(x[LD_PSR]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetGATE_VECTOR(int *x)   { return(x[GATE_VECTOR]); }
int GetPSRMUX(int *x)        { return((x[PSRMUX1] << 1) + x[PSRMUX0]); }
int GetOPRMUX(int *x)        { return(x[OPRMUX]); }
int GetR6MUX(int *x)         { return((x[R6MUX1] << 1) + x[R6MUX0]); }
int GetCCMUX(int *x)         { return(x[CCMUX]); }
int GetI_RESET(int *x)       { return(x[I_RESET]); }
int GetE_RESET(int *x)       { return(x[E_RESET]); }

int GetV2P(int *x)           { return(x[V2P]); }
int GetLD_VA(int *x)         { return(x[LD_VA]); }
int GetLD_J(int *x)          { return(x[LD_J]); }
int GetGATE_PTBR(int *x)     { return(x[GATE_PTBR]); }
int GetGATE_VA(int *x)       { return(x[GATE_VA]); }
int GetCK_PROT_PAGE(int *x)  { return(x[CK_PROT_PAGE]); }
int GetUPDATE_PTE(int *x)    { return(x[UPDATE_PTE]); }


/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */ 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

  int PC,		/* program counter */
	  MDR,	/* memory data register */
	  MAR,	/* memory address register */
	  IR,		/* instruction register */
	  N,		/* n condition bit */
	  Z,		/* z condition bit */
	  P,		/* p condition bit */
	  BEN;        /* ben register */

  int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
	 at a bad point in the cycle*/

  int REGS[LC_3b_REGS]; /* register file. */

  int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

  int STATE_NUMBER; /* Current State Number - Provided for debugging */ 

  /* For lab 4 */
  int INTV; /* Interrupt vector register */
  int EXCV; /* Exception vector register */
  int SSP; /* Initial value of system stack pointer */
  /* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */
  int USP; /* Temporarily store user stack pointer */
  int PSR; /* Processor Status Register */
  int E; /* Exception Detetion Flag, used in microsequencer */
  int I; /* Interrupt Detection Flag, used in microsequencer */

  int TEMP_J;

  /* For lab 5 */
  int PTBR; /* This is initialized when we load the page table */
  int VA;   /* Temporary VA register */
  /* MODIFY: you should add here any other registers you need to implement virtual memory */

} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3bSIM Help-------------------------\n");
  printf("go               -  run program to completion       \n");
  printf("run n            -  execute program for n cycles    \n");
  printf("mdump low high   -  dump memory from low to high    \n");
  printf("rdump            -  dump the register & bus values  \n");
  printf("?                -  display this help menu          \n");
  printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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
  fflush(dumpsim_file);
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

  printf("USP           : 0x%0.4x\n", CURRENT_LATCHES.USP);
  printf("SSP           : 0x%0.4x\n", CURRENT_LATCHES.SSP);
  printf("PSR           : 0x%0.4x\n", CURRENT_LATCHES.PSR);
  printf("INTV           : 0x%0.4x\n", CURRENT_LATCHES.INTV);
  printf("EXCV           : 0x%0.4x\n", CURRENT_LATCHES.EXCV);
  printf("I           : 0x%0.4x\n", CURRENT_LATCHES.I);
  printf("E           : 0x%0.4x\n", CURRENT_LATCHES.E);
  printf("TEMP_J         : 0x%0.4x\n", CURRENT_LATCHES.TEMP_J);
  printf("PTBR         : 0x%0.4x\n", CURRENT_LATCHES.PTBR);
  printf("VA           : 0x%0.4x\n", CURRENT_LATCHES.VA);



  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Cycle Count  : %d\n", CYCLE_COUNT);
  printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
  printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
  printf("BUS          : 0x%0.4x\n", BUS);
  printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
  printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
  fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
  fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
  fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
  fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
  fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
  FILE *ucode;
  int i, j, index;
  char line[200];

  printf("Loading Control Store from file: %s\n", ucode_filename);

  /* Open the micro-code file. */
  if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
  }

  /* Read a line for each row in the control store. */
  for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	  printf("Error: Too few lines (%d) in micro-code file: %s\n",
		  i, ucode_filename);
	  exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	  /* Needs to find enough bits in line. */
	  if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
			ucode_filename, i);
		exit(-1);
	  }
	  if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
			ucode_filename, i, j);
		exit(-1);
	  }

	  /* Set the bit in the Control Store. */
	  CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	  index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	  printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		  ucode_filename, i);
  }
  printf("\n");
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
void load_program(char *program_filename, int is_virtual_base) {                   
  FILE * prog;
  int ii, word, program_base, pte, virtual_pc;

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

  if (is_virtual_base) {
	if (CURRENT_LATCHES.PTBR == 0) {
	  printf("Error: Page table base not loaded %s\n", program_filename);
	  exit(-1);
	}

	/* convert virtual_base to physical_base */
	virtual_pc = program_base << 1;
	pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) | 
	  MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

	printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
	if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	  program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
	  printf("physical base of program: %x\n\n", program_base);
	  program_base = program_base >> 1; 
	} else {
	  printf("attempting to load a program into an invalid (non-resident) page\n\n");
	  exit(-1);
	}
  }
  else {
	/* is page table */
	CURRENT_LATCHES.PTBR = program_base << 1;
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
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
	ii++;
  }

  if (CURRENT_LATCHES.PC == 0 && is_virtual_base) 
	CURRENT_LATCHES.PC = virtual_pc;

  printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *pagetable_filename, char *program_filename, int num_prog_files) { 
  int i;
  init_control_store(ucode_filename);

  init_memory();
  load_program(pagetable_filename,0);
  for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename,1);
	while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;
  CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
  memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
  CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

  /* MODIFY: you can add more initialization code HERE */
  CURRENT_LATCHES.USP = 0xFE00; /* Initial value of user stack pointer */
  CURRENT_LATCHES.PSR = 0x8000 + (CURRENT_LATCHES.N << 2) + (CURRENT_LATCHES.Z << 1) + (CURRENT_LATCHES.P); /* Initial value of processor status register: privilege level + cc */
  CURRENT_LATCHES.E = 0;
  CURRENT_LATCHES.I = 0;

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
  if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
		argv[0]);
	exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argv[2], argv[3], argc - 3);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
  }

  while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated 
   with a "MODIFY:" comment.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/


void eval_micro_sequencer() {

  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   */
  int J, COND, COND2, and4, and2, and1, and0, J5, J4, J3, J2, J1, J0;

  /* Add a control signal to the mux, so if E==1, the micro sequencer will always choose the next state as 49 */
  if (CURRENT_LATCHES.E == 1) {
	NEXT_LATCHES.STATE_NUMBER = Low16bits(0x31);
  } else {

	COND2 = GetCOND2(CURRENT_LATCHES.MICROINSTRUCTION);
	and4 = COND2 & (CURRENT_LATCHES.I);
	if (GetV2P(CURRENT_LATCHES.MICROINSTRUCTION) && !and4 ) {
	  NEXT_LATCHES.STATE_NUMBER = Low16bits(0x36);
	  NEXT_LATCHES.TEMP_J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
	} else {
	  if (GetIRD(CURRENT_LATCHES.MICROINSTRUCTION)) {
		NEXT_LATCHES.STATE_NUMBER = ((CURRENT_LATCHES.IR) >> 12) & 0xF;
	  } else {
		if (!GetLD_J(CURRENT_LATCHES.MICROINSTRUCTION)) {
		  J = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
		} else {
		  J = CURRENT_LATCHES.TEMP_J;
		}
		COND = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
		/* if(and4) printf("Come into State 49!!\n"); */
		and2 = ((COND >> 1) & 0x1) & !(COND & 0x1) & CURRENT_LATCHES.BEN;
		and1 = !((COND >> 1) & 0x1) & (COND & 0x1) & CURRENT_LATCHES.READY;
		and0 = ((COND >> 1) & 0x1) & (COND & 0x1) & ((CURRENT_LATCHES.IR >> 11) & 0x1);
		J5 = (J >> 5) & 0x1;
		J4 = (J >> 4) & 0x1;
		J3 = (J >> 3) & 0x1;
		J2 = (J >> 2) & 0x1;
		J1 = (J >> 1) & 0x1;
		J0 = (J) & 0x1;
		NEXT_LATCHES.STATE_NUMBER = (J5 << 5) + ((J4 | and4) << 4) + (J3 << 3) + ((J2 | and2) << 2) + ((J1 | and1) << 1) + (J0 | and0);
	  }
	}
  }

  /* printf("READY:%d\n", CURRENT_LATCHES.READY); */
  memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

}


int memory_cycle = 0;

void cycle_memory() {

  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
  if (!memory_cycle) {
	if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  memory_cycle++;
	}
  } else {
	if (memory_cycle == 3) {
	  NEXT_LATCHES.READY = 1;
	  memory_cycle++;
	} else if (memory_cycle == 4) {
	  NEXT_LATCHES.READY = 0;
	  memory_cycle = 0;
	} else {
	  memory_cycle++;
	}
  }

  printf("memory_cycle:%d\n", memory_cycle);
  
  if (CYCLE_COUNT == 300) {
	printf("Interruption!!\n");
	NEXT_LATCHES.I = 1;
	NEXT_LATCHES.INTV = 0x01;
  }

}

enum BUS_DRIVER {
  GATE_MARMUX_DRIVER,
  GATE_PC_DRIVER,
  GATE_ALU_DRIVER,
  GATE_SHF_DRIVER,
  GATE_MDR_DRIVER,
  GATE_PSR_DRIVER,
  GATE_VECTOR_DRIVER,
  GATE_PTBR_DRIVER,
  GATE_VA_DRIVER,
  EMPTY_DRIVER,
} BUS_DRIVER;
int bus_driver;

void eval_bus_drivers() {

  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR,
   *		 Gate_PSR,
   *		 Gate_VECTOR,
   *		 Gate_PTBR,
   *		 Gate_VA,
   */    
  if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_MARMUX_DRIVER;
  } else if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_PC_DRIVER;
  } else if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_ALU_DRIVER;
  } else if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_SHF_DRIVER;
  } else if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_MDR_DRIVER;
  } else if (GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_PSR_DRIVER;
  } else if (GetGATE_VECTOR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_VECTOR_DRIVER;
  } else if (GetGATE_PTBR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_PTBR_DRIVER;
  } else if (GetGATE_VA(CURRENT_LATCHES.MICROINSTRUCTION)) {
	bus_driver = GATE_VA_DRIVER;
  } else {
	bus_driver = EMPTY_DRIVER;
  }
}


/* This function is used for sign extending a number with digits bits */
int sext(int num, int digits) {
  int sign = (num >> (digits - 1)) & 0x1;
  if (!sign) {
	return num;
  } else {
	return num - (1 << digits);
  }
}


/* x is the offset */
#define GETREG(x) (CURRENT_LATCHES.REGS[((CURRENT_LATCHES.IR) >> (x)) & 0x7]); /* 0b111 */
/* x is the bit number */
#define GETNUM(x) ((CURRENT_LATCHES.IR) & ((1 << (x)) - 1))


/* Implement the logic for the MUX under MARMUX and PCMUX, with Base+Offset */
int AddrAdditor() {
  int SR1, BASE, OFFSET;
  /* ???????????????DOUBLE check whether we should concat/cut off OFFSET into 16 bits???? */
  switch (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	case 0:
	  OFFSET = 0;
	  break;
	case 1:
	  OFFSET = sext(GETNUM(6), 6);
	  break;
	case 2:
	  OFFSET = sext(GETNUM(9), 9);
	  break;
	case 3:
	  OFFSET = sext(GETNUM(11), 11);
	  break;
	default:
	  printf("impossible...");
	  OFFSET = 0;
	  break;
  }
  /* DO we need to cut off OFFSET?????????????*/
  if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION)) {
	OFFSET = OFFSET << 1;
  }
  if (!GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	BASE = CURRENT_LATCHES.PC;
  } else {
	/*Actually we don't need LD_R6 switch here......*/
	if (GetLD_R6(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  SR1 = CURRENT_LATCHES.REGS[6];
	} else {
	  if (!GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		SR1 = GETREG(9);
	  } else {
		SR1 = GETREG(6); 
	  }
	}
	BASE = SR1;
  }
  return Low16bits(BASE + OFFSET);
}


void drive_bus() {

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */       
  int SR1, SR2, OP1, OP2, steer;
  if (bus_driver == GATE_MARMUX_DRIVER) {
	/*????????? DOUBLE CHECK here??? No sign extension*/
	if(!GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  BUS = Low16bits((unsigned)GETNUM(8) << 1);
	} else {
	  BUS = AddrAdditor();
	}
  } else if (bus_driver == GATE_PC_DRIVER) {
	BUS = CURRENT_LATCHES.PC;
  } else if (bus_driver == GATE_ALU_DRIVER) {
	if (GetLD_R6(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  SR1 = CURRENT_LATCHES.REGS[6];
	} else {
	  if (!GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		SR1 = GETREG(9);
	  } else {
		SR1 = GETREG(6); 
	  }
	}
	if (((CURRENT_LATCHES.IR) >> 5) & 0x1) {
	  SR2 = sext(GETNUM(5), 5);
	} else {
	  SR2 = GETREG(0);
	}
	/* printf("SR1: %d; SR2: %d\n", SR1, SR2); */
	switch(GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  case 0:
		BUS = Low16bits(SR1 + SR2);
		break;
	  case 1:
		BUS = Low16bits(SR1 & SR2);
		break;
	  case 2:
		BUS = Low16bits(SR1 ^ SR2);
		break;
	  case 3:
		BUS = Low16bits(SR1);
		break;
	  default:
		printf("impossible...\n");
		BUS = 0;
		break;
	}
  } else if (bus_driver == GATE_SHF_DRIVER) {
	/*Actually we don't need LD_R6 switch here......*/
	if (GetLD_R6(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  SR1 = CURRENT_LATCHES.REGS[6];
	} else {
	  if (!GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		SR1 = GETREG(9);
	  } else {
		SR1 = GETREG(6); 
	  }
	}
	OP1 = GETNUM(4);
	steer = ((CURRENT_LATCHES.IR) >> 4) & 0x3;
	switch(steer) {
	  case 0:
		BUS = Low16bits(SR1 << OP1);
		break;
	  case 1:
		BUS = Low16bits((unsigned int)SR1 >> OP1);
		break;
		/* ??????????????????????????NEED MORE TEST>>>>>>>>>>>*/
	  case 3:
		/***********CHECK sext(SR1, 16) *****/
		BUS = Low16bits(sext(SR1, 16) >> OP1);
		break;
	  default:
		printf("impossible...\n");
		BUS = 0;
		break;
	}
  } else if (bus_driver == GATE_MDR_DRIVER) {
	if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  BUS = CURRENT_LATCHES.MDR;
	} else {
	  /* State 31, DR<-SEXT[BYTE.DATA] setcc */
	  if (CURRENT_LATCHES.MAR & 0x1) {
		BUS = Low16bits(sext((Low16bits(CURRENT_LATCHES.MDR) >> 8) & 0xFF,8));
	  } else {
		BUS = Low16bits(sext(0xFF & (CURRENT_LATCHES.MDR), 8));
	  }
	}
  } else if (bus_driver == GATE_PSR_DRIVER) {
	BUS = Low16bits(CURRENT_LATCHES.PSR);
  } else if (bus_driver == GATE_VECTOR_DRIVER) {
	if (CURRENT_LATCHES.I != 0) {
	  BUS = Low16bits(0x0200 + (CURRENT_LATCHES.INTV << 1));
	} else if (CURRENT_LATCHES.EXCV != 0) {
	  BUS = Low16bits(0x0200 + (CURRENT_LATCHES.EXCV << 1));
	} else {
	  BUS = 0;
	  /*???????????????????????????????????????????*/
	  /* printf("There should be no output here.....impossible...\n"); */
	}
  } else if (bus_driver == GATE_PTBR_DRIVER) {
	/*0b1111111*/
	BUS = Low16bits(CURRENT_LATCHES.PTBR + (((CURRENT_LATCHES.VA >> 9) & 0x7F) << 1));
  } else if (bus_driver == GATE_VA_DRIVER) {
	int mar = 0, pfn;
	pfn = ((CURRENT_LATCHES.MDR >> 9) & 0x1F);
	mar |= (pfn << 9);
	mar |= (CURRENT_LATCHES.VA & 0x1FF);
	/*CURRENT_LATCHES.MAR = mar;*/
	BUS = Low16bits(mar);
  } else if (bus_driver == EMPTY_DRIVER) {
	BUS = 0;
  }
}


/* Set the condition number according the bus */
void setcc(int res) {
  int sign;
  if (!res) {
	NEXT_LATCHES.N = 0;
	NEXT_LATCHES.Z = 1;
	NEXT_LATCHES.P = 0;
  } else {
	sign = (res >> 15) & 0x1;
	if (!sign) {
	  NEXT_LATCHES.N = 0;
	  NEXT_LATCHES.Z = 0;
	  NEXT_LATCHES.P = 1;
	} else {
	  NEXT_LATCHES.N = 1;
	  NEXT_LATCHES.Z = 0;
	  NEXT_LATCHES.P = 0;
	}
  }
}

/* Set the condition number according to PSR */
void setccPSR(int PSR) {
  NEXT_LATCHES.N = (PSR >> 2) & 0x1;
  NEXT_LATCHES.Z = (PSR >> 1) & 0x1;
  NEXT_LATCHES.P = (PSR) & 0x1;
}



#define GETBIT(x) (((CURRENT_LATCHES.IR) >> (x)) & 0x1)
#define MEMBYTE(x) MEMORY[(Low16bits(x)) >> 1][(Low16bits(x)) & 0x1]
#define MEMWORD(x) (Low16bits(((MEMORY[(Low16bits(x)) >> 1][1]) << 8) + ((MEMORY[(Low16bits(x)) >> 1][0]) & 0xFF)))

void latch_datapath_values() {

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
  /* LD MAR */
  if (GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.MAR = Low16bits(BUS);
	if (GetLD_E(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  /* HERE NEXT_LATCHES.MAR = Low16bits(BUS); */
	  /*
	  if (((CURRENT_LATCHES.PSR >> 15) & 0x1) && (((BUS >> 12) & 0xF) <= 0x2)) {
		NEXT_LATCHES.E = 1;
		NEXT_LATCHES.EXCV = 0x02;
		printf("Protection Exception!\n");
	  } else { */
		if ((CURRENT_LATCHES.STATE_NUMBER != 2) && (CURRENT_LATCHES.STATE_NUMBER != 3)) {
		  if (BUS & 0x1) {
			NEXT_LATCHES.E = 1;
			NEXT_LATCHES.EXCV = 0x03;
			printf("Unaligned Access Exception!\n");
		  }
		} 
		/*else {
		  if (GetCK_PROT_PAGE(CURRENT_LATCHES.MICROINSTRUCTION)) {
			if ()
			  CK_PROT_PAGE: can only be WORD, MDR == BUS
		  }
		}*/
/*    } */
      
	}
  } 

  /* LD MDR */
  if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	if (!GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION)) { /* Read from BUS */
	  if (!GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) { /* BYTE */
		/*
		if (CURRENT_LATCHES.MAR & 0x1) {
		  CURRENT_LATCHES.MDR = BUS && 0x00FF;
		} else {
		}
		*/
		/* State 24 , MDR <- SR[7:0] */
		NEXT_LATCHES.MDR = ((BUS & 0xFF) << 8) + (BUS & 0xFF);
	  } else { /* WORD */
		NEXT_LATCHES.MDR = Low16bits(BUS);
	  }
	} else { /* Read from Memory */
	  if (CURRENT_LATCHES.READY) {
		NEXT_LATCHES.MDR = MEMWORD(CURRENT_LATCHES.MAR);
	  } else {
		NEXT_LATCHES.MDR = CURRENT_LATCHES.MDR;
	  }
	}
	if (GetLD_E(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  /*impossible to have both protection and page fault: all pages in system space resident in the physical memory*/
	  if (GetCK_PROT_PAGE(CURRENT_LATCHES.MICROINSTRUCTION)) {
	    if (((CURRENT_LATCHES.PSR >> 15) & 0x1) && (!((BUS >> 3) & 0x1))) {
		  NEXT_LATCHES.E = 1;
		  NEXT_LATCHES.EXCV = 0x04;
		  printf("Protection Exception!\n");
		} 
		if (!((BUS >> 2) & 0x1)) {
		  NEXT_LATCHES.E = 1;
		  NEXT_LATCHES.EXCV = 0x02;
		  printf("Page Fault!\n");
		}
	  }
	}
  } 

  /* LD IR */
  if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.IR = Low16bits(BUS);
  } 

  /* LD BEN */
  if (GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.BEN = Low16bits((GETBIT(11) & (CURRENT_LATCHES.N)) || (GETBIT(10) & (CURRENT_LATCHES.Z)) || (GETBIT(9) & (CURRENT_LATCHES.P)));
	if (GetLD_E(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  int opcode;
	  opcode = (CURRENT_LATCHES.IR >> 12) & 0xF;
	  if (opcode == 10 || opcode == 11) {
		NEXT_LATCHES.E = 1;
		NEXT_LATCHES.EXCV = 0x05;
		printf("Unknown Opcode Exception!\n");
	  }
	}
  }

  /* LD REG */
  if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION)) {
	if (GetLD_R6(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  switch(GetR6MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		case 0:
		  if (!GetOPRMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
			NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.REGS[6] - 2);
		  } else {
			NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.REGS[6] + 2);
		  }
		  break;
		case 1:
		  NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.SSP);
		  break;
		case 2:
		  NEXT_LATCHES.REGS[6] = Low16bits(CURRENT_LATCHES.USP);
		  break;
		default:
		  /*only 3 cases???????????????????????*/
		  printf("impossible.....\n");
		  break;
	  }
	} else {
	  if (!GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
		/* printf("R%d\n", ((CURRENT_LATCHES.IR) >> 9) & 0x7); */
		NEXT_LATCHES.REGS[((CURRENT_LATCHES.IR) >> 9) & 0x7] = Low16bits(BUS);
	  } else {
		NEXT_LATCHES.REGS[7] = Low16bits(BUS);
	  }
	}
  }

  /* LD CC */
  if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION)) {
	if (!GetCCMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  setcc(BUS);
	} else {
	  setccPSR(BUS);
	}
  } 

  /* LD PC */
  if (GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {
    switch (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  case 0:
		NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
		break;
	  case 1:
		NEXT_LATCHES.PC = Low16bits(BUS);
		break;
	  case 2:
		NEXT_LATCHES.PC = AddrAdditor();
		break;
	  case 3:
		NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC - 2);
		break;
	  default:
		printf("impossible...");
		break;
	}
  }

  /* Memory Enable && Write*/
  if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) && GetR_W(CURRENT_LATCHES.MICROINSTRUCTION)) {
	if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  MEMBYTE(CURRENT_LATCHES.MAR) = Low16bits(CURRENT_LATCHES.MDR & 0xFF);
	  MEMBYTE(CURRENT_LATCHES.MAR + 1) = Low16bits((CURRENT_LATCHES.MDR >> 8) & 0xFF);
	} else {
	  /* State 17 */
	  if (CURRENT_LATCHES.MAR & 0x1) {
		MEMBYTE(CURRENT_LATCHES.MAR) = (CURRENT_LATCHES.MDR >> 8) & 0xFF;
	  } else {
		MEMBYTE(CURRENT_LATCHES.MAR) = CURRENT_LATCHES.MDR & 0xFF;
	  }
	}
  }

  /* LD SSP */
  if (GetLD_SSP(CURRENT_LATCHES.MICROINSTRUCTION)) {
	/* printf("LD_SSP...\n"); */
	NEXT_LATCHES.SSP = BUS;
  } 

  /* LD USP */
  if (GetLD_USP(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.USP = BUS;
  } 

  /* LD PSR */
  if (GetLD_PSR(CURRENT_LATCHES.MICROINSTRUCTION)) {
	switch(GetPSRMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
	  case 0:
		NEXT_LATCHES.PSR = Low16bits(BUS);
		break;
	  case 1:
		NEXT_LATCHES.PSR = Low16bits(CURRENT_LATCHES.PSR & 0x7FFF);
		break;
		/*????????????????????double check??????????????????????????*/
	  case 2:
		NEXT_LATCHES.PSR = (CURRENT_LATCHES.PSR & 0xFFF8) + (NEXT_LATCHES.N << 2) + (NEXT_LATCHES.Z << 1) + (NEXT_LATCHES.P);
		break;
	  default:
		printf("impossible\n");
		break;
	}
  }

  /* RESET I */
  if (GetI_RESET(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.I = 0;
  } 

  /* RESET E */
  if (GetE_RESET(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.E = 0;
  } 

  if (GetLD_VA(CURRENT_LATCHES.MICROINSTRUCTION)) {
	NEXT_LATCHES.VA = Low16bits(CURRENT_LATCHES.MAR);
  }

  if (GetUPDATE_PTE(CURRENT_LATCHES.MICROINSTRUCTION)) {
	int opcode = (CURRENT_LATCHES.IR >> 12) & 0xF;
	NEXT_LATCHES.MDR = Low16bits(CURRENT_LATCHES.MDR | 0x01);
	/* If the pending access is a write, set the modified bit of the PTE*/
	if (opcode == 3 || opcode == 7) {
	  NEXT_LATCHES.MDR = Low16bits(CURRENT_LATCHES.MDR | 0x02);
	}
  }


}
