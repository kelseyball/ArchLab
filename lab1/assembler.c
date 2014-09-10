#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<limits.h>

#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define MAX_LINE_LENGTH 255
#define OPCODE_SIZE 28

#define IMM5_MAX 15
#define IMM5_MIN -16

#define OFFSET6_MAX 31
#define OFFSET6_MIN -32



const char *opcode[] = {"add", "and", "br", "brn", "brz", "brp", "brnp", "brnz", "brzp", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "rti", "lshf", "rshfl", "rshfa", "stb", "stw", "trap", "xor"};

/*
 * brn, brp, brnp, br, brz, brnz, brzp, brnzp; How to handle with it?
 */

enum {
  DONE, OK, EMPTY_LINE
};

typedef struct {
  int address;
  char label[MAX_LABEL_LEN + 1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];
int symbolTableSize;

typedef struct{
  char opcode[5];
  int encoding;
}OpcodeEncodingMap;

OpcodeEncodingMap opcodeEncodingMap[OPCODE_SIZE] = {
  {"add",   0b0001},
  {"and",   0b0101},
  {"br",    0b0000},
  {"brn",   0b0000},
  {"brp",   0b0000},
  {"brz",   0b0000},
  {"brnp",  0b0000},
  {"brnz",  0b0000},
  {"brzp",  0b0000},
  {"brnzp", 0b0000},
  {"halt",  0b1111},
  {"jmp",   0b1100},
  {"jsr",   0b0100},
  {"jsrr",  0b0100},
  {"ldb",   0b0010},
  {"ldw",   0b0110},
  {"lea",   0b1110},
  {"nop",   0b0000},
  {"not",   0b1001},
  {"ret",   0b1100},
  {"rti",   0b1000},
  {"lshf",  0b1101},
  {"rshfl", 0b1101},
  {"rshfa", 0b1101},
  {"stb",   0b0011},
  {"stw",   0b0111},
  {"trap",  0b1111},
  {"xor",   0b1001},
};

int encodeOpcode(char *opcode) {
  int i;
  for (i = 0; i < OPCODE_SIZE; ++i) {
	if (!strcmp(opcodeEncodingMap[i].opcode, opcode)) {
	  return opcodeEncodingMap[i].encoding;
	}
  }
  printf("error");
  return -1;
  //error....
}

FILE* infile = NULL;
FILE* outfile = NULL;

int isOpcode(char *ptr) {
  int i;
  for (i = 0; i < OPCODE_SIZE; ++i) {
	if (strcmp(ptr, opcode[i]) == 0) {
	  return 0;
	}
  }
  return -1;
}


int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, int *pArgc, char **pArg) {
  char *lRet, *lPtr;
  int i;

  *pArgc = 0;
  if (!fgets(pLine, MAX_LINE_LENGTH, pInfile)) {
	return (DONE);
  }
  for (i = 0; i < strlen(pLine); ++i) {
	pLine[i] = tolower(pLine[i]);
  }

  *pLabel = *pOpcode = pArg[0] = pArg[1] = pArg[2] = pArg[3] = pLine + strlen(pLine);/*What's the purpose? all is '\0' now ?*/

  lPtr = pLine;

  while (*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n') {
	lPtr++;
  }

  *lPtr = '\0';
  if (!(lPtr = strtok(pLine, "\t\n ,"))) {
	return (EMPTY_LINE);
  }

  if (isOpcode(lPtr) == -1 && lPtr[0] != '.') {
	*pLabel = lPtr;
	if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);
  }
  *pOpcode = lPtr;

  if (strcmp(*pOpcode, ".end") == 0) {
	/*No operands after .end is allowed...............NEED to ask the TA................*/
	/*
    if ((lPtr = strtok(NULL, "\t\n ,"))) {
      printf("wrong number of operand\n");	  
	  exit(4);
	}
	*/
	return (DONE);
  }

  while (1) {
    if (!(lPtr = strtok(NULL, "\t\n ,"))) {return (OK);}
    pArg[*pArgc] = lPtr;
    (*pArgc)++;
	if (*pArgc >= 4) {
	  printf("Error: too many operands\n");
	}
  }
  return (OK);
}

void firstPass(char *iFileName) {
  char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg[4];
  int lRet, lArgc;
  int startAddr;

  FILE *lInfile;
  int symbolTableIndex = 0;
  int lineNo = 0;
  int i;

  lInfile = fopen(iFileName, "r");
  do {
	lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArgc, lArg);
	if (lRet != DONE && lRet != EMPTY_LINE) {
      printf("line %d: %s\n", lineN, lLine);
	  if (lineNo == 0) {
		/*
		if (*lLabel != '\0') {
		  printf("Error: no label for the first line before .ORIG, %s\n", orig_pStr);
	      exit(4);
		}
		*/
		if (strcmp(lOpcode, ".orig") != 0) {
		  printf("Error: The first line should contains .ORIG\n");
	      exit(4);
		}

		printf("%s\n", lArg[0]);
		printf("%d\n", lArgc);

		if (lArgc == 1) {
		  startAddr = toNum(lArg[0]);
		  printf ("%d\n", startAddr);
		} else {
		  printf("Error: Lacking the first operand or adding extra additional operands\n");
		  exit(4);
		}

	  }

	  if (*lLabel != '\0') {
		//printf("come inside the label branch\n");
		symbolTable[symbolTableIndex].address = startAddr + lineNo * 2;
		strcpy(symbolTable[symbolTableIndex].label, lLabel);
		symbolTableIndex++;
	  }
	  lineNo++;
	}

  } while (lRet != DONE);

  for (i = 0; i < symbolTableIndex; ++i) {
	printf("addr:%d; label:%s\n", symbolTable[i].address, symbolTable[i].label);
  }
  fclose(lInfile);

}

int isReg(char *ptr) {
  int regID;
  if (strlen(ptr) != 2) {
	return 0;
  }
  if (ptr[0] == 'r' && isdigit(ptr[1])) {	
	regID = atoi(ptr+1);
	if (regID >= 0 && regID <= 8) {
	  return 1;
	} else {
	  /*error msg is shown by toNum....*/
	  return 0;
	}
  } else {
	  /*error msg is shown by toNum....*/
	return 0;
  }
}


int extractRegID(char *ptr, int lineNo) {
  int regID;
  printf("reg:%s\n", ptr);
  if (strlen(ptr) != 2) {
	errorMsg(1, lineNo);
  }
  if (ptr[0] == 'r' && isdigit(ptr[1])) {	
	regID = atoi(ptr+1);
	if (regID >= 0 && regID <= 7) {
	  return regID;
	} else {
	  errorMsg(1, lineNo);
	}
  } else {
	//error....
	//printf();
  }
}
/*
int isImm(char *ptr) {
  //we can directly use to Num
}
*/

void errorMsg(int errorNo, int lineNo) {
  switch (errorNo):
  case 0:
      printf("Error: wrong number of operands: %d", lineNo);
      exit(4);
	  break;
  case 1:
      printf("Error: Invalid oprand: %d", lineNo);
      exit(4);
	  break;
  case 2:
      printf("Error: Invalid constant: %d", lineNo);
      exit(3);
	  break;
  case 3:
 


  default:
      printf("Error: wrong number of operands: %d", lineNo);
      exit(4);
}


void genHexCode(char *pLabel, char *pOpcode, int pArgc, char **pArg, FILE *lOutfile, int lineNo) {
  int number;
  int lInstr = 0;

  lInstr |= (encodeOpcode(pOpcode) << 12); /*encode opcode*/
  if (!strcmp(pOpcode, "add") || !strcmp(pOpcode, "and") || !strcmp(pOpcode, "xor")) {
    /*check the number of operands is 3*/
    if (pArgc != 3) {
	  errorMsg(0, lineNo);
    }
    lInstr |= (extractRegID(pArg[0]) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1]) << 6); /*encode SR1*/
    if (isReg(pArg[2])) { /*encode SR2 or imm5*/
      lInstr |= 0 << 5;
      lInstr |= 00 << 3;
      lInstr |= (extractRegID(pArg[2]));
    } else {
      lInstr |= 1 << 5;
      //we need to check whether lArg2 has exceeded the largest possible number???
      number = toNum(pArg[2]);
      //Do we need to check this??????????????????
      if (number > IMM5_MAX || number < IMM5_MIN) {
		errorMsg(2, lineNo);
      }
      lInstr |= (number & 0b11111);
    }
  } else if (strcmp(pOpcode, "br") == 0) {
  
  } else if (!strcmp(pOpcode, "ldb") || !strcmp(pOpcode, "ldw") || !strcmp(pOpcode, "stb") || !strcmp(pOpcode, "stw")) {
    /*check the number of operands is 3*/
    if (pArgc != 3) {
	  errorMsg(0, lineNo);
    }
    lInstr |= (extractRegID(pArg[0]) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1]) << 6); /*encode SR1*/
	number = toNum(pArg[2]);
	if (number > OFFSET6_MAX || number < OFFSET6_MIN) {
	  errorMsg(2, lineNo);
	}
	lInstr |= (number & 0b111111);
  } else if (!strcmp(pOpcode, "lshf") || !strcmp(pOpcode, "rshfl") || !strcmp(pOpcode, "rshfa")) {
    if (pArgc != 3) {
	  errorMsg(0, lineNo);
    }
    lInstr |= (extractRegID(pArg[0]) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1]) << 6); /*encode SR1*/

	if(!strcmp(pOpcode, "lshf")) {
	}

	number = toNum(pArg[2]);
	
 

  }

 
 
  
  }
  
  fprintf(lOutfile, "0x%.4X\n", lInstr);

}

void secondPass(char *iFileName, char *oFileName) {
  char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg[4];
  int lRet, lArgc;
  int startAddr;

  FILE *lInfile, *lOutfile;
  int symbolTableIndex = 0;
  int lineNo = 0;
  int i, temp;

  lInfile = fopen(iFileName, "r");
  lOutfile = fopen(oFileName, "w");
  do {
	lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArgc, lArg);
	if (lRet != DONE && lRet != EMPTY_LINE) {
      printf("line %d: %s\n", lineNo, lLine);

	  genHexCode(lLabel, lOpcode, lArgc, lArg, lOutfile, lineNo);

	}
	lineNo++;

  } while (lRet != DONE);

  printf("%s\n", lLabel);

  /*
  if (strcmp(lLabel, ".end") != 0) {
	printf("Error: The last line should be \".END\"\n");
	exit(4);
  }
  */

  fclose(lInfile);
}


int main(int argc, char* argv[]) {
  char *prgName = NULL;
  char *iFileName = NULL;
  char *oFileName = NULL;

  if (argc != 3) {
	printf("Usage: ./assemble <source.asm> <output.target>\n");
	exit(4);
  }
  /* arvn ?? != 3, error.... */
  prgName = argv[0];
  iFileName = argv[1];
  oFileName = argv[2];

  printf("program name = '%s'\n", prgName);
  printf("inputfile name = '%s'\n", iFileName);
  printf("output file name = '%s'\n", oFileName);

  firstPass(iFileName);

  secondPass(iFileName, oFileName);

  infile = fopen(argv[1], "r");
  outfile = fopen(argv[2], "w");

  if (!infile) {
	printf("Error: Cannot open file %s\n", argv[1]);
	exit(4);
  }

  if (!outfile) {
	printf("Error: Cannot open file %s\n", argv[2]);
	exit(4);
  }

  fclose(infile);
  fclose(outfile);
  
}

int toNum(char * pStr ) {
  char * t_ptr;
  char * orig_pStr;
  int t_length, k;
  int lNum, lNeg = 0;
  long int lNumLong;

  orig_pStr = pStr;
  if (*pStr == '#') {
	pStr++;
	if(*pStr == '-') {
	  lNeg = 1;
	  pStr++;
	}
	t_ptr = pStr;
	t_length = strlen(t_ptr);
	for (k = 0; k < t_length; ++k) {
	  if (!isdigit(*t_ptr)) {
		printf("Error: invalid decimal operand, %s\n", orig_pStr);
		exit(4);
	  }
	  t_ptr++;
	}
	lNum = atoi(pStr);
	if (lNeg) {
	  lNum = -lNum;
	}
	return lNum;
  } else if (*pStr == 'x') {
	pStr++;
	if (*pStr == '-') {
	  lNeg = 1;
	  pStr++;
	}
	t_ptr = pStr;
	t_length = strlen(t_ptr);
	for (k = 0; k < t_length; ++k) {
	  if (!isxdigit(*t_ptr)) {
		printf("Error: invalid hex operand, %s\n", orig_pStr);
	  }
	  t_ptr++;
	}
	lNumLong = strtol(pStr, NULL, 16);
	lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong; /*Any errors here? I guess too large should exit as error*/
	if (lNeg) {
	  lNum = -lNum;
	}
	return lNum;
  } else {
	printf("Error: invalid operand, %s\n", orig_pStr);
	exit(4);
  }
}














