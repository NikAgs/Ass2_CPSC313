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

struct Instr {
	unsigned int iCd : 4;
	unsigned int iFn : 4;
	unsigned int rA : 4;
	unsigned int rB : 4;
	unsigned long long C;
	bool endOfFile = false;
	bool error = false;
}

int main(int argc, char **argv) {

	FILE *machineCode, *outputFile;
	long currAddr = 0;
	unsigned char currByte;
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

struct Instr readInstr(FILE *machinecode) {
	struct Instr currInstr;
	int currByte;

	if((currByte = fgetc(machinecode)) != EOF) {
		currInstr.iCd = currByte >> 4;
		currInstr.iFn = currByte & 0xf;
	} else {
		currInstr.end = true;
		return currInstr;
	}
	
	switch(currInstr.iCd) {
		case I_HALT:
		case I_NOP:
		case I_IRMOVQ:
		case I_RMMOVQ:
		case I_MRMOVQ:
		case I_CALL:
		case I_RET:
		case I_PUSHQ:
		case I_POPQ:
			if(currInstr.iFn != 0x0) {
				currInstr.error = true;
				return currInstr;
			}
			break;
		case I_RRMOVQ:
		case I_JXX:
			switch(currInstr.iFn) {
				case C_NC:
				case C_LE:
				case C_L:
				case C_E:
				case C_NE:
				case C_GE:
				case C_G:

			}
			break;
		case I_OPQ:
			switch(currInstr.iFn) {
				case A_ADDQ:
				case A_SUBQ:
				case A_ANDQ:
				case A_XORQ:
				case A_MULQ:
				case A_DIVQ:
				case A_MODQ:
			}
			break;
	}
	return currInstr
}
