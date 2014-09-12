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

#define AMOUNT4_MAX 15
#define AMOUNT4_MIN 0

#define TRAPVECT8_MAX 255
#define TRAPVECT8_MIN 0

#define FILL_MAX 65535
//#define FILL_MIN -65536
#define FILL_MIN -32768


#define ORIG_MAX 65535
#define ORIG_MIN 0

#define PC9_MAX 255
#define PC9_MIN -256

#define PC11_MAX 1023
#define PC11_MIN -1024


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


int startAddr;

void errorMsg(int errorNo, int lineNo) {
  switch (errorNo) {
  case 0:
      printf("Error: wrong number of operands: %d\n", lineNo);
      exit(4);
	  break;
  case 1:
      printf("Error: Invalid operand: %d\n", lineNo);
      exit(4);
	  break;
  case 2:
      printf("Error: Invalid constant: %d\n", lineNo);
      exit(3);
	  break;
  case 3:
      printf("Error: Undefined label: %d\n", lineNo);
      exit(1);
	  break;
  case 4:
      printf("Error: Invalid opcode: %d\n", lineNo);
      exit(2);
	  break;
  case 5:
	  printf("Error: Invalid label: %d\n", lineNo);
	  exit(4);
	  break;
  default:
      printf("Error: %d\n", lineNo);
      exit(4);
  }
}


int extractAddr(char *label, int lineNo) {
  int i;
  printf("label: %s\n", label);
  for (i = 0; i < symbolTableSize; ++i) {
	if (!strcmp(symbolTable[i].label, label)) {
	  return symbolTable[i].address;
	}
  }
  errorMsg(3, lineNo);
}

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

int encodeOpcode(char *opcode, int lineNo) {
  int i;
  for (i = 0; i < OPCODE_SIZE; ++i) {
	if (!strcmp(opcodeEncodingMap[i].opcode, opcode)) {
	  return opcodeEncodingMap[i].encoding;
	}
  }
  printf("error");
  errorMsg(4, lineNo); 
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


int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode, int *pArgc, char **pArg, int lineNo) {
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

  /*
  if (strcmp(*pOpcode, ".end") == 0) {
	//No operands after .end is allowed...............NEED to ask the TA................
    if ((lPtr = strtok(NULL, "\t\n ,"))) {
      printf("wrong number of operand\n");	  
	  exit(4);
	}
	return (DONE);
  }
  */
  while (1) {
    if (!(lPtr = strtok(NULL, "\t\n ,"))) {return (OK);}
    pArg[*pArgc] = lPtr;
    (*pArgc)++;
	if (*pArgc >= 4) {
	  errorMsg(0, lineNo);
	}
  }
  return (OK);
}

void checkLabel(char *label, int lineNo) {
  int i;
  /* Do we need to check if label is an opcode? since we get label only when label is not an opcode */
  if (!strcmp(label, "in") || !strcmp(label, "out") || !strcmp(label, "getc") || !strcmp(label, "puts") || isOpcode(label) != -1) errorMsg(5, lineNo);
  if (*label == 'x') errorMsg(5, lineNo);
  for (i = 0; label[i] != '\0'; ++i) {
	if (!isalnum(label[i])) errorMsg(5, lineNo);
  }
  return;
}

void firstPass(char *iFileName) {
  char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg[4];
  int lRet, lArgc;

  FILE *lInfile;
  int symbolTableIndex = 0;
  int lineNo = 0;
  int i;

  lInfile = fopen(iFileName, "r");
  if (!lInfile) {
	printf("Error: Cannot open file\n");
	exit(4);
  }
  do {
	lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArgc, lArg, lineNo);
	if (lRet != DONE && lRet != EMPTY_LINE) {
      printf("line %d: %s\n", lineNo, lLine);
	  if (lineNo == 0) {
		if (*lLabel != '\0') {
		  printf("Error: no label for the first line before .ORIG\n");
	      exit(4);
		}

		if (strcmp(lOpcode, ".orig") != 0) {
		  printf("Error: The first line should contains .ORIG\n");
	      exit(4);
		}

		printf("%s\n", lArg[0]);
		printf("%d\n", lArgc);

		if (lArgc == 1) {
		  startAddr = toNum(lArg[0]);
		  printf ("%d\n", startAddr);
		  if (startAddr % 2) errorMsg(2, lineNo); 
		} else {
		  printf("Error: Lacking the first operand or adding extra additional operands\n");
		  exit(4);
		}

	  }

	  if (*lLabel != '\0') {
		//printf("come inside the label branch\n");
		//symbolTable[symbolTableIndex].address = startAddr + lineNo * 2;
		checkLabel(lLabel, lineNo);

		symbolTable[symbolTableIndex].address = lineNo;
		strcpy(symbolTable[symbolTableIndex].label, lLabel);
		symbolTableIndex++;
	  }
	  lineNo++;
	}

  } while (lRet != DONE);

  for (i = 0; i < symbolTableIndex; ++i) {
	printf("addr:%d; label:%s\n", symbolTable[i].address, symbolTable[i].label);
  }
  symbolTableSize = symbolTableIndex;
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
	  printf("hello");
	  errorMsg(1, lineNo);
	}
  } else {
	  errorMsg(1, lineNo);
  }
}
/*
int isImm(char *ptr) {
  //we can directly use to Num
}
*/

int genHexCode(char *pLabel, char *pOpcode, int pArgc, char **pArg, FILE *lOutfile, int lineNo) {
  int number, address;
  int lInstr = 0;

  if (pOpcode[0] != '.') lInstr |= (encodeOpcode(pOpcode, lineNo) << 12); /*encode opcode*/
  if (!strcmp(pOpcode, "add") || !strcmp(pOpcode, "and") || !strcmp(pOpcode, "xor")) {
    if (pArgc != 3) errorMsg(0, lineNo); /* check the number of operands is 3 */
    lInstr |= (extractRegID(pArg[0], lineNo) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1], lineNo) << 6); /*encode SR1*/
    if (isReg(pArg[2])) { /*encode SR2 or imm5*/
      lInstr |= 0b0 << 5;
      lInstr |= 0b00 << 3;
      lInstr |= (extractRegID(pArg[2], lineNo));
    } else {
      lInstr |= 0b1 << 5;
      number = toNum(pArg[2]);
      if (number > IMM5_MAX || number < IMM5_MIN) {
		errorMsg(2, lineNo);
      }
      lInstr |= (number & 0b11111);
    }
  } else if (!strcmp(pOpcode, "ldb") || !strcmp(pOpcode, "ldw") || !strcmp(pOpcode, "stb") || !strcmp(pOpcode, "stw")) {
    if (pArgc != 3) errorMsg(0, lineNo); /* check the number of operands is 3 */
    lInstr |= (extractRegID(pArg[0], lineNo) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1], lineNo) << 6); /*encode SR1*/
	number = toNum(pArg[2]);
	if (number > OFFSET6_MAX || number < OFFSET6_MIN) errorMsg(2, lineNo);
	lInstr |= (number & 0b111111);
  } else if (!strcmp(pOpcode, "lshf") || !strcmp(pOpcode, "rshfl") || !strcmp(pOpcode, "rshfa")) {
    if (pArgc != 3) errorMsg(0, lineNo); /* check the number of operands is 3 */
    lInstr |= (extractRegID(pArg[0], lineNo) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1], lineNo) << 6); /*encode SR1*/
	if(!strcmp(pOpcode, "lshf")) lInstr |= (0b00 << 4);
	else if(!strcmp(pOpcode, "rshfl")) lInstr |= (0b01 << 4);
	else if(!strcmp(pOpcode, "rshfa")) lInstr |= (0b10 << 4);
	number = toNum(pArg[2]);
	if (number > AMOUNT4_MAX || number < AMOUNT4_MIN) errorMsg(2, lineNo);
	lInstr |= (number & 0b1111);
  } else if(!strcmp(pOpcode, "not")) {
    if (pArgc != 2) errorMsg(0, lineNo); /* check the number of operands is 2 */
	lInstr |= (extractRegID(pArg[0], lineNo) << 9); /*encode DR*/
    lInstr |= (extractRegID(pArg[1], lineNo) << 6); /*encode SR1*/
	lInstr |= 0b1 << 5;
	lInstr |= 0b11111;
  } else if(!strcmp(pOpcode, "jmp")) {
    if (pArgc != 1) errorMsg(0, lineNo); /* check the number of operands is 1 */
	lInstr |= (0b000 << 9);
    lInstr |= (extractRegID(pArg[0], lineNo) << 6); /*encode BaseR*/
	lInstr |= 0b000000;
  } else if(!strcmp(pOpcode, "trap")) {
    if (pArgc != 1) errorMsg(0, lineNo); /* check the number of operands is 1 */
	lInstr |= (0b1111 << 12);
	number = toNum(pArg[0]);
	if (number > TRAPVECT8_MAX || number < TRAPVECT8_MIN) errorMsg(2, lineNo);
	lInstr |= (number & 0b11111111);
  } else if(!strcmp(pOpcode, "br") || !strcmp(pOpcode, "brn") || !strcmp(pOpcode, "brz") || !strcmp(pOpcode, "brp") || !strcmp(pOpcode, "brnz") || !strcmp(pOpcode, "brnp") || !strcmp(pOpcode, "brzp") || !strcmp(pOpcode, "brnzp")) {
    if (pArgc != 1) errorMsg(0, lineNo); /* check the number of operands is 1 */

	if(!strcmp(pOpcode, "br")) lInstr |= (0b111 << 9);
	else if(!strcmp(pOpcode, "brn")) lInstr |= (0b100 << 9);
	else if(!strcmp(pOpcode, "brz")) lInstr |= (0b010 << 9);
	else if(!strcmp(pOpcode, "brp")) lInstr |= (0b001 << 9);
	else if(!strcmp(pOpcode, "brnz")) lInstr |= (0b110 << 9);
	else if(!strcmp(pOpcode, "brnp")) lInstr |= (0b101 << 9);
	else if(!strcmp(pOpcode, "brzp")) lInstr |= (0b011 << 9);
	else if(!strcmp(pOpcode, "brnzp")) lInstr |= (0b111 << 9);
	
	/* if (pArg[0][0] = '#' || pArg[0][0] == 'x') errorMsg(1, lineNo); */
	address = extractAddr(pArg[0], lineNo);
	number = address - lineNo - 1; /* incremented PC... */

	/* if (number > PC9_MAX || number < PC9_MIN) errorMsg(2, lineNo); */
	lInstr |= (number & 0b111111111);

  } else if(!strcmp(pOpcode, "lea")) {
    if (pArgc != 2) errorMsg(0, lineNo); /* check the number of operands is 2 */
    lInstr |= (extractRegID(pArg[0], lineNo) << 9); /*encode DR*/

	/* if (pArg[0][0] = '#' || pArg[0][0] == 'x') errorMsg(1, lineNo); */
	address = extractAddr(pArg[1], lineNo);
	number = address - lineNo - 1; /* incremented PC... */

	/* if (number > PC9_MAX || number < PC9_MIN) errorMsg(2, lineNo); */
	lInstr |= (number & 0b111111111);

  } else if(!strcmp(pOpcode, "jsr")) {
    if (pArgc != 1) errorMsg(0, lineNo); /* check the number of operands is 1 */
	lInstr |= (0b1 << 11);

	/* if (pArg[0][0] = '#' || pArg[0][0] == 'x') errorMsg(1, lineNo); */
	address = extractAddr(pArg[0], lineNo);
	number = address - lineNo - 1; /* incremented PC... */

	/* if (number > PC11_MAX || number < PC11_MIN) errorMsg(2, lineNo); */
	lInstr |= (number & 0b11111111111);

  } else if(!strcmp(pOpcode, "jsrr")) {
	lInstr |= (0b0 << 11);
	lInstr |= (0b00 << 9);
    lInstr |= (extractRegID(pArg[0], lineNo) << 6); /*encode BaseR*/
	lInstr |= (0b000000);

  } else if(!strcmp(pOpcode, "halt")) {
    if (pArgc != 0) errorMsg(0, lineNo); /* check the number of operands is 0 */
	lInstr = 0b1111000000100101;
  } else if(!strcmp(pOpcode, "nop")) {
    if (pArgc != 0) errorMsg(0, lineNo); /* check the number of operands is 0 */
	lInstr = 0b0000000000000000;
  } else if(!strcmp(pOpcode, "ret")) {
    if (pArgc != 0) errorMsg(0, lineNo); /* check the number of operands is 0 */
	lInstr = 0b1100000111000000;
  } else if(!strcmp(pOpcode, "rti")) {
    if (pArgc != 0) errorMsg(0, lineNo); /* check the number of operands is 0 */
	lInstr = 0b1000000000000000;
  } else if(!strcmp(pOpcode, ".fill")) {
    if (pArgc != 1) errorMsg(0, lineNo); /* check the number of operands is 1 */
	number = toNum(pArg[0]);
	if (number > FILL_MAX || number < FILL_MIN) errorMsg(2, lineNo);
	lInstr |= (number & 0b1111111111111111);
  } else if (!strcmp(pOpcode, ".orig")) {
	if (lineNo != 0) errorMsg(4, lineNo);
    if (pArgc != 1) errorMsg(0, lineNo); /* check the number of operands is 1 */
	number = toNum(pArg[0]);
	if (number > ORIG_MAX || number < ORIG_MIN || number % 2) errorMsg(2, lineNo);
	lInstr |= (number & 0b1111111111111111);
  } else if (!strcmp(pOpcode, ".end")) {
    if (pArgc != 0) errorMsg(0, lineNo); /* check the number of operands is 0 ...................Do we need to check this???? .end cannot have any operands..... */
	return 1;
  } else {
	errorMsg(4, lineNo);
  }
  
  fprintf(lOutfile, "0x%.4X\n", lInstr);
  return 0;
}

void secondPass(char *iFileName, char *oFileName) {
  char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg[4];
  int lRet, lArgc;

  FILE *lInfile, *lOutfile;
  int symbolTableIndex = 0;
  int lineNo = 0;
  int i, temp;

  lInfile = fopen(iFileName, "r");
  lOutfile = fopen(oFileName, "w");
  if (!lInfile) {
	printf("Error: Cannot open file\n");
	exit(4);
  }
  if (!lOutfile) {
	printf("Error: Cannot open file\n");
	exit(4);
  }
  do {
	lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArgc, lArg, lineNo);
	if (lRet != DONE && lRet != EMPTY_LINE) {
      printf("sec line %d: %s\n", lineNo, lLine);
	  if (genHexCode(lLabel, lOpcode, lArgc, lArg, lOutfile, lineNo))
		break;
	  lineNo++;
	}
  } while (lRet != DONE);

  printf("lOpcode: %s\n", lOpcode);

  /* check the last line is ".end" or not, we can also use a flag to judge the return value of genHexCode.... */
  if (strcmp(lOpcode, ".end") != 0) {
	printf("Error: The last line should be \".END\"\n");
	exit(4);
  }

  fclose(lInfile);
  fclose(lOutfile);
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

  /*
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
  */
  
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




