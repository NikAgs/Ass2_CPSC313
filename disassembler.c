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

typedef enum { false, true } bool;

struct Instr {
    long addr;
    long fullHex;
    char name;
    char op1;
    char op2;
    bool error;
    bool endFile;
};

int printInstr(struct Instr instr) {

    //printf("%08x: %-14s", addr, fullHex, );
    //n=snprintf(buff, 256, "%08x: %-14s%-8s  %s, %s\n", currAddr, buffer, "rrmovl", regA, regB);
    //res = fileno(f);
    //write(res, buff, n );
}


int main(int argc, char **argv) {

    FILE *machineCode, *outputFile;
    long currAddr = 0;
    unsigned char currByte;

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

    samplePrint(stderr);

    fclose(machineCode);
    fclose(outputFile);
    return SUCCESS;
}

struct Instr readInstr(FILE *machinecode) {
    struct Instr currInstr;
    unsigned int iCd,iFn;
    int currByte;

    if((currByte = fgetc(machinecode)) != EOF) {
        iCd = currByte >> 4;
        iFn = currByte & 0xf;
    } else {
        return currInstr;
    }

    switch(iCd) {
        case I_HALT:
            break;
        case I_NOP:
            break;
        case I_IRMOVQ:
            break;
        case I_RMMOVQ:
            break;
        case I_MRMOVQ:
            break;
        case I_CALL:
            break;
        case I_RET:
            break;
        case I_PUSHQ:
            break;
        case I_POPQ:
            if(iFn != 0x0) {
                currInstr.error = true;
                return currInstr;
            }
            break;
        case I_RRMOVQ:
            break;
        case I_JXX:
            switch(iFn) {
                case C_NC:
                    break;
                case C_LE:
                    break;
                case C_L:
                    break;
                case C_E:
                    break;
                case C_NE:
                    break;
                case C_GE:
                    break;
                case C_G:
                    break;
            }
            break;
        case I_OPQ:
            switch(iFn) {
                case A_ADDQ:
                    break;
                case A_SUBQ:
                    break;
                case A_ANDQ:
                    break;
                case A_XORQ:
                    break;
                case A_MULQ:
                    break;
                case A_DIVQ:
                    break;
                case A_MODQ:
                    break;
            }
            break;
    }
    return currInstr;
}