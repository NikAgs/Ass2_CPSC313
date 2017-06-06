#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "printRoutines.h"

#define ERROR_RETURN -1
#define SUCCESS 0

#define I_HALT 0x0
#define I_NOP 0x1
#define I_RRMOVQ 0x2
#define I_IRMOVQ 0x3
#define I_RMMOVQ 0x4
#define I_MRMOVQ 0x5
#define I_OPQ 0x6
#define I_JXX 0x7
#define I_CALL 0x8
#define I_RET 0x9
#define I_PUSHQ 0xa
#define I_POPQ 0xb

#define A_ADDQ 0x0
#define A_SUBQ 0x1
#define A_ANDQ 0x2
#define A_XORQ 0x3
#define A_MULQ 0x4
#define A_DIVQ 0x5
#define A_MODQ 0x6

#define C_NC 0x0
#define C_LE 0x1
#define C_L 0x2
#define C_E 0x3
#define C_NE 0x4
#define C_GE 0x5
#define C_G 0x6

bool error = false;
bool endFile = false;

struct Instr {
    long addr;
    char fullHex[11];
    char name[7];
    char op1[15];
    char op2[15];
};

int main(int argc, char **argv) {

	FILE *machineCode, *outputFile;
	long currAddr = 0;
	struct Instr currInstr;

	// Verify that the command line has an appropriate number
	// of arguments

	if (argc < 3 || argc > 4) {
		printf("Usage: %s InputFilename OutputFilename [startingOffset]\n", argv[0]);
		return ERROR_RETURN;
	}

	// First argument is the file to read, attempt to open it 
	// for reading and verify that the open did occur.
	machineCode = fopen(argv[1], "rb");

	if (machineCode == NULL) {
		printf("Failed to open %s: %s\n", argv[1], strerror(errno));
		return ERROR_RETURN;
	}

	// Second argument is the file to write, attempt to open it 
	// for writing and verify that the open did occur.
	outputFile = fopen(argv[2], "w");

	if (outputFile == NULL) {
		printf("Failed to open %s: %s\n", argv[2], strerror(errno));
		fclose(machineCode);
		return ERROR_RETURN;
	}

	// If there is a 3rd argument present it is an offset so
	// convert it to a value. 
	if (4 == argc) {
		// See man page for strtol() as to why
		// we check for errors by examining errno
		errno = 0;
		currAddr = strtol(argv[3], NULL, 0);
		if (errno != 0) {
			perror("Invalid offset on command line");
			fclose(machineCode);
			fclose(outputFile);
			return ERROR_RETURN;
		}
	}

	printf("Opened %s, starting offset 0x%lX\n", argv[1], currAddr);
	printf("Saving output to %s\n", argv[2]);

	fclose(machineCode);
	fclose(outputFile);
	return SUCCESS;
}

struct Instr readInstr(FILE *machineCode, long addr) {
	struct Instr currInstr;
	int currByte;
	char iCd;
	char iFn;
	char rA;
	char rB;

	//Start stream from offset
	fseek(machineCode, addr, SEEK_SET);

	//Set address of instruction
	currInstr.addr = addr;

	//Read first byte of instruction
	//First byte is always iCd/iFn
	//Always check if end of file
	if((currByte = fgetc(machineCode)) != EOF) {
		currInstr.fullHex[0] = (char) currByte;
		iFn = currByte >> 4;
		iCd = currByte & 0xf;
	} else {
		endFile = true;
		return currInstr;
	}
	
	//Check if iCd and iFn working. Only halt, nop, and ret are finished here
	switch(iCd) {

		//HALT, NOP, and RET only have iFn and iCd so we return regardless of error
		case I_HALT:
			currInstr.name = "halt";
			currInstr.fullHex[1] = '\0';
			if(currInstr.iFn != 0x0) {
				error = true;
			}
			return currInstr;
			break;
		case I_NOP:
			currInstr.name = "nop";
			currInstr.fullHex[1] = '\0';
			if(currInstr.iFn != 0x0) {
				error = true;
			}
			return currInstr;
			break;
		case I_RET:
			currInstr.name = "ret";
			currInstr.fullHex[1] = '\0';
			if(currInstr.iFn != 0x0) {
				error = true;
			}
			return currInstr;
			break;
		case I_RRMOVQ:
			switch(iFn) {
				case C_NC:
					currInstr.name = "rrmovq";
					break;
				case C_LE:
					currInstr.name = "cmovle";
					break;
				case C_L:
					currInstr.name = "cmovl";
					break;
				case C_E:
					currInstr.name = "cmove";
					break;
				case C_NE:
					currInstr.name = "cmovne";
					break;
				case C_GE:
					currInstr.name = "cmovge";
					break;
				case C_G:
					currInstr.name = "cmovg";
					break;
				default:
					error = true;
					return currInstr;
			}

			if((currByte = fgetc(machineCode)) != EOF) {
				currInstr.fullHex[1] = (char) currByte;
				currInstr.fullHex[2] = '\0';
				rA = currByte >> 4;
				rB = currByte & 0xf;
				currInstr.op1 = getRegister(rA);
				currInstr.op2 = getRegister(rB);
				if(error == true) {
					return currInstr;
				}
			} else {
				endFile = true;
				return currInstr;
			}
			break;
		case I_OPQ:
			switch(iFn) {
				case A_ADDQ:
					currInstr.name = "andq";
					break;
				case A_SUBQ:
					currInstr.name = "subq";
					break;
				case A_ANDQ:
					currInstr.name = "andq";
					break;
				case A_XORQ:
					currInstr.name = "xorq";
					break;
				case A_MULQ:
					currInstr.name = "mulq";
					break;
				case A_DIVQ:
					currInstr.name = "divq";
					break;
				case A_MODQ:
					currInstr.name = "modq";
					break;
				defualt:
					error = true;
					return currInstr;
			}

			if((currByte = fgetc(machineCode)) != EOF) {
				currInstr.fullHex[1] = (char) currByte;
				currInstr.fullHex[2] = '\0';
				rA = currByte >> 4;
				rB = currByte & 0xf;
				currInstr.op1 = getRegister(rA);
				currInstr.op2 = getRegister(rB);
				if(error == true) {
					return currInstr;
				}
			} else {
				endFile = true;
				return currInstr;
			}
			break;
		case I_PUSHQ:
			if(currInstr.iFn != 0x0) {
				error = true;
				return currInstr;
			}

			currInstr.name = "pushq";
			if((currByte = fgetc(machineCode)) != EOF) {
				currInstr.fullHex[1] = (char) currByte;
				currInstr.fullHex[2] = '\0';
				rA = currByte >> 4;
				currInstr.op1 = getRegister(rA);
				if(error == true) {
					return currInstr;
				}
			} else {
				endFile = true;
				return currInstr;
			}
			break;
		case I_POPQ:
			if(currInstr.iFn != 0x0) {
				error = true;
				return currInstr;
			}

			currInstr.name = "popq";
			if((currByte = fgetc(machineCode)) != EOF) {
				currInstr.fullHex[1] = (char) currByte;
				currInstr.fullHex[2] = '\0';
				rA = currByte >> 4;
				currInstr.op1 = getRegister(rA);
				if(error == true) {
					return currInstr;
				}
			} else {
				endFile = true;
				return currInstr;
			}
			break;
//------------------------------------------------------- Finishline
		case I_IRMOVQ:
			if(currInstr.iFn != 0x0) {
				error = true;
				return currInstr;
			}

			currInstr.name = "irmovq";
			if((currByte = fgetc(machineCode)) != EOF) {
				currInstr.fullHex[1] = (char) currByte;
				rB = currByte & 0xf;
				currInstr.op2 = getRegister(rB);
				if(error == true) {
					return currInstr;
				}
			} else {
				endFile = true;
				return currInstr;
			}

			for(int i = 0; i < 8; i++) {
				if((currByte = fgetc(machineCode)) != EOF) {
					currInstr.fullHex[i + 2] = (char) currByte;

			break;
		case I_RMMOVQ:
		case I_MRMOVQ:
		case I_CALL:

		case I_JXX:
			switch(currInstr.iFn) {
				case C_NC:
				case C_LE:
				case C_L:
				case C_E:
				case C_NE:
				case C_GE:
				case C_G:
					break;
				default:
					error = true;
					return currInstr;
			}
			break;
		//iCd is broken if here
		defualt:
			error = true;
			return currInstr;
	}
}

char[] getRegister(char r) {
	switch(r) {
		case 0x0:
			return "%rax";
			break;
		case 0x1:
			return "%rcx";
			break;
		case 0x2:
			return "%rdx";
			break;
		case 0x3:
			return "%rbx";
			break;	
		case 0x4:
			return "%rsp";
			break;
		case 0x5:
			return "%rbp";
			break;
		case 0x6:
			return "%rsi";
			break;
		case 0x7:
			return "%rdi";
			break;
		case 0x8:
			return "%r8";
			break;
		case 0x9:
			return "%r9";
			break;
		case 0xa:
			return "%r10";
			break;
		case 0xb:
			return "%r11";
			break;
		case 0xc:
			return "%r12";
			break;
		case 0xd:
			return "%r13";
			break;
		case 0xe:
			return "%r14";
			break;
		default:
			error = true;
			return "error";
	}
}
