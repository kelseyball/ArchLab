#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<limits.h>

#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define MAX_LINE_LENGTH 255
#define OPCODE_SIZE 21

const char *opcode[] = {"add", "and", "br", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor"};

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

FILE* infile = NULL;
FILE* outfile = NULL;



int isOpcode(char *ptr) {
  int i;
  for (i = 0; i < OPCODE_SIZE; ++i) {
	if (strcmp(ptr, opcode[i]) == 0) {
	  return 1;
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
      printf("line %d: %s\n", lineNo, lLine);
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
  if (ptr[0] == 'R' && isdigit(ptr[1])) {	
	regID = atoi(ptr+1);
	if (regID >= 0 && regID <= 8) {
	  return 1;
	} else {
	  return 0;
	}
  } else {
	return 0;
  }
}


int extractRegID(char *ptr) {
  int regID;
  printf("reg:%s\n", ptr);
  if (strlen(ptr) != 2) {
	//error...
  }
  if (ptr[0] == 'r' && isdigit(ptr[1])) {	
	regID = atoi(ptr+1);
	if (regID >= 0 && regID <= 8) {
	  printf("coming inside regid\n");

	  return regID;
	} else {
	  //error;
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


void secondPass(char *iFileName, char *oFileName) {
  char lLine[MAX_LINE_LENGTH+1], *lLabel, *lOpcode, *lArg[4];
  int lRet, lArgc;
  int startAddr;
  int lInstr = 0;

  FILE *lInfile, *lOutfile;
  int symbolTableIndex = 0;
  int lineNo = 0;
  int i, temp;

  lInfile = fopen(iFileName, "r");
  lOutfile = fopen(oFileName, "w");
  do {
	lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArgc, lArg);
	if (lRet != DONE && lRet != EMPTY_LINE) {
	  lInstr = 0;
      printf("line %d: %s\n", lineNo, lLine);

	  if (strcmp(lOpcode, "add") == 0) {
		//check the number of operands is 3..
		if (lArgc != 3) {
		  printf("Error: wrong number of operands");
		  exit(4);
		}
		lInstr |= (0001 << 12); /*encode opcode*/
		lInstr |= (extractRegID(lArg[0]) << 9); /*encode DR*/
		lInstr |= (extractRegID(lArg[1]) << 6); /*encode SR1*/
		if (isReg(lArg[2])) { /*encode SR2 or imm5*/
		  lInstr |= 0 << 5;
		  lInstr |= 00 << 3;
		  lInstr |= (extractRegID(lArg[2]));
		} else {
	      lInstr |= 1 << 5;
		  //we need to check whether lArg2 has exceeded the largest possible number???
		  temp = toNum(lArg[2]);
		  if (temp > 15 || temp < -16) {
			printf("Error: Out of range\n");
			exit(4);
		  }
		  lInstr |= (temp & 0b11111 );
		}
	  } else if (strcmp(lOpcode, "and") == 0) {

	  } else if (strcmp(lOpcode, "br") == 0) {

	  } else if (strcmp(lOpcode, "") == 0) {

	  }

	  fprintf(lOutfile, "0x%.4X\n", lInstr);
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














