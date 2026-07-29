#define JSM
#ifdef JSM



#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>


#define BYTECODE_STATEMENT_SIZE 24


#define REGISTER_COUNT 255


#define TRUE 1
#define FALSE 0
#define NONE 0
#define JSM_OK 1
#define JSM_ERROR 0



enum INTRUCTIONS_ENUM
{

        EXIT,
        RETURN,
        END,
        JUMP,
        SKIP,
        SETMODE,
        SET,
        ADD,
        SUB,
        MUL,
        DIV,
        CMPE,
        CMPH,
        CMPL,
        CMPHE,
        CMPLE,
        PUSH,
        POP,
        LOAD,
        SYSCALL,

};


enum REGISTERS_ENUM
{

        RSP,
        RSB,
        RDP,
        RDB,
        RLA,
        RLP,
        RRA,
        RRV,
        RSM,
        RS1,
        RS2,
        RS3,
        RG1,
        RG2,
        RG3,
        RG4,
        RG5,
        RG6,
        RG7,
        RG8,
        RG9,

};


#define REG 1
#define VAL 0

#define OPERAND_1_TYPE_INDEX 0
#define OPERAND_2_TYPE_INDEX 1



int JSM__READ_FILE_TO_JSMCODE(const char* FILE_PATH, size_t* JSMCODE_LENGTH, char** JSMCODE);


int JSM__CHECK_BYTECODE_SIZE(char* JSMCODE, const size_t JSMCODE_LENGTH, size_t* BYTECODE_SIZE);


int JSM__COMPILE_TO_BYTECODE(char* JSMCODE, char* BYTECODE, const size_t JSMCODE_LENGTH, size_t* BYTECODE_SIZE);


int JRM__RUN(const char* CODE, const size_t BYTECODE_SIZE, const size_t STACK_SIZE_MB);



#endif
