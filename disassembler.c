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

char *getRegister(char r);
struct Instr readInstr(FILE *machineCode, long addr);
typedef enum { false, true } bool;

bool error = false;
bool endFile = false;

struct Instr {
    long addr;
    char fullHex[100];
    char name[100];
    char op1[100];
    char op2[100];
};

int printInstr(FILE *out, struct Instr instr) {

    if (!error) {
        printf("%016lx: ", instr.addr);

        for (int i=0; i<20; i++) {
            if (instr.fullHex[i] == '\0') {
                printf(" ");
            } else {
                printf("%c", instr.fullHex[i]);
            }
        }

        printf("  %s", instr.name);
        if  (strlen(instr.op1) != 0) {
            printf(" %s", instr.op1);
        } 
        if (strlen(instr.op2) != 0) {
            printf(", %s", instr.op2);
        }
        printf("\n");
        return SUCCESS;
    } else {
        return ERROR_RETURN;
    }
}

char *getRegister(char r) {
    char *ret = malloc(6);
    switch(r) {
        case 0x0:
            strcpy(ret, "%rax");
            break;
        case 0x1:
            strcpy(ret, "%rcx");
            break;
        case 0x2:
            strcpy(ret, "%rdx");
            break;
        case 0x3:
            strcpy(ret, "%rbx");
            break;	
        case 0x4:
            strcpy(ret, "%rsp");
            break;
        case 0x5:
            strcpy(ret, "%rbp");
            break;
        case 0x6:
            strcpy(ret, "%rsi");
            break;
        case 0x7:
            strcpy(ret, "%rdi");
            break;
        case 0x8:
            strcpy(ret, "%r8");
            break;
        case 0x9:
            strcpy(ret, "%r9");
            break;
        case 0xa:
            strcpy(ret, "%r10");
            break;
        case 0xb:
            strcpy(ret, "%r11");
            break;
        case 0xc:
            strcpy(ret, "%r12");
            break;
        case 0xd:
            strcpy(ret, "%r13");
            break;
        case 0xe:
            strcpy(ret, "%r14");
            break;
        default:
            error = true;
            strcpy(ret, "error");
    }
    return ret;
}

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


    printInstr(outputFile, readInstr(machineCode, currAddr));


    fclose(machineCode);
    fclose(outputFile);
    return SUCCESS;
}

struct Instr readInstr(FILE *machineCode, long addr) {
    struct Instr currInstr;
    int currByte;
    int iCd;
    int iFn;
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
        //currInstr.fullHex[0] = (char) currByte;
        iCd = currByte >> 4;
        iFn = currByte & 0xf;
        sprintf(currInstr.fullHex, "%02X", currByte);
        memset(currInstr.fullHex+2, '\0', 25);
    } else {
        endFile = true;
        return currInstr;
    }

    //Check if iCd and iFn working. Only halt, nop, and ret are finished here
    switch(iCd) {
            //HALT, NOP, and RET only have iFn and iCd so we return regardless of error
        case I_HALT:
            strcpy(currInstr.name, "halt");
            currInstr.op1[0] = '\0';
            currInstr.op2[0] = '\0';
            if(iFn != 0x0) {
                error = true;
            }
            return currInstr;
            break;

        case I_NOP:
            strcpy(currInstr.name, "nop");
            currInstr.op1[0] = '\0';
            currInstr.op2[0] = '\0';
            if(iFn != 0x0) {
                error = true;
            }
            return currInstr;
            break;

        case I_RET:
            strcpy(currInstr.name, "ret");
            currInstr.op1[0] = '\0';
            currInstr.op2[0] = '\0';
            if(iFn != 0x0) {
                error = true;
            }
            return currInstr;
            break;

        case I_RRMOVQ:
            switch(iFn) {
                case C_NC:
                    strcpy(currInstr.name, "rrmovq");
                    break;
                case C_LE:
                    strcpy(currInstr.name, "cmovle");
                    break;
                case C_L:
                    strcpy(currInstr.name, "cmovl");
                    break;
                case C_E:
                    strcpy(currInstr.name, "cmove");
                    break;
                case C_NE:
                    strcpy(currInstr.name, "cmovne");
                    break;
                case C_GE:
                    strcpy(currInstr.name, "cmovge");
                    break;
                case C_G:
                    strcpy(currInstr.name, "cmovg");
                    break;
                default:
                    error = true;
                    return currInstr;
            }
            if((currByte = fgetc(machineCode)) != EOF) {
                sprintf(currInstr.fullHex+2, "%02X", currByte);
                rA = currByte >> 4;
                rB = currByte & 0xf;
                strcpy(currInstr.op1, getRegister(rA));
                strcpy(currInstr.op2, getRegister(rB));
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
                    strcpy(currInstr.name, "addq");
                    break;
                case A_SUBQ:
                    strcpy(currInstr.name, "subq");
                    break;
                case A_ANDQ:
                    strcpy(currInstr.name, "andq");
                    break;
                case A_XORQ:
                    strcpy(currInstr.name, "xorq");
                    break;
                case A_MULQ:
                    strcpy(currInstr.name, "mulq");
                    break;
                case A_DIVQ:
                    strcpy(currInstr.name, "divq");
                    break;
                case A_MODQ:
                    strcpy(currInstr.name, "modq");
                    break;
                default:
                    error = true;
                    return currInstr;
            }
            if((currByte = fgetc(machineCode)) != EOF) {
                sprintf(currInstr.fullHex+2, "%02X", currByte);
                rA = currByte >> 4;
                rB = currByte & 0xf;
                strcpy(currInstr.op1, getRegister(rA));
                strcpy(currInstr.op2, getRegister(rB));
                if(error == true) {
                    return currInstr;
                }
            } else {
                endFile = true;
                return currInstr;
            }
            break;

        case I_PUSHQ:
            if(iFn != 0x0) {
                error = true;
                return currInstr;
            }

            strcpy(currInstr.name, "pushq");
            if((currByte = fgetc(machineCode)) != EOF) {
                sprintf(currInstr.fullHex+2, "%02X", currByte);
                rA = currByte >> 4;
                strcpy(currInstr.op1, getRegister(rA));
                currInstr.op2[0] = '\0';
                if(error == true) {
                    return currInstr;
                }
            } else {
                endFile = true;
                return currInstr;
            }
            break;

        case I_POPQ:
            if(iFn != 0x0) {
                error = true;
                return currInstr;
            }

            strcpy(currInstr.name, "popq");
            if((currByte = fgetc(machineCode)) != EOF) {
                sprintf(currInstr.fullHex+2, "%02X", currByte);
                rA = currByte >> 4;
                strcpy(currInstr.op1, getRegister(rA));
                currInstr.op2[0] = '\0';
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
            if(iFn != 0x0) {
                error = true;
                return currInstr;
            }

            strcpy(currInstr.name, "irmovq");
            if((currByte = fgetc(machineCode)) != EOF) {
                currInstr.fullHex[1] = (char) currByte;
                rB = currByte & 0xf;
                strcpy(currInstr.op2, getRegister(rB));
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
                }
            }
            break;

        case I_RMMOVQ:
            break;

        case I_MRMOVQ:
            break;

        case I_CALL:
            break;

        case I_JXX:
            switch(iFn) {
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
    }

    return currInstr;
}
