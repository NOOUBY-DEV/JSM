#include "../JSM.h"
#include <stddef.h>
#include <stdio.h>



#define SIZE_CHECK_MODE 0
#define COMPILE_MODE 1



typedef struct INTRUCTION__STRING_TO_REQUIRED_OPERANDS
{

        char* STRING;
        unsigned char REQUIRED_OPERANDS;

}
INTRUCTION__STRING_TO_REQUIRED_OPERANDS;




const INTRUCTION__STRING_TO_REQUIRED_OPERANDS INSTRUCTION_STRINGS_LIST[] =
{

        {"EXIT", 1},
        {"RETURN", 0},
        {"END", 0},
        {"JUMP", 1},
        {"SKIP", 0},
        {"SETMODE", 2},
        {"SET", 2},
        {"ADD", 2},
        {"SUB", 2},
        {"MUL", 2},
        {"DIV", 2},
        {"CMPE", 2},
        {"CMPH", 2},
        {"CMPL", 2},
        {"CMPHE", 2},
        {"CMPLE", 2},
        {"PUSH", 2},
        {"POP", 2},
        {"LOAD", 2},
        {"SYSCALL", 0},
        {NULL, NONE}


};


const char* REGISTER_STRINGS[] =
{

        "RSP",
        "RSB",
        "RDP",
        "RDB",
        "RLA",
        "RLP",
        "RRA",
        "RRV",
        "RSM",
        "RS1",
        "RS2",
        "RS3",
        "RG1",
        "RG2",
        "RG3",
        "RG4",
        "RG5",
        "RG6",
        "RG7",
        "RG8",
        "RG9",
        NULL

};


typedef struct MACRO__STRING_TO_VALUE__STRING_TO_VALUE
{

        char* STRING;
        unsigned long VALUE;

}
MACRO__STRING_TO_VALUE;


const MACRO__STRING_TO_VALUE MACRO__STRING_TO_VALUE__LIST[] =
{

        {"REG", 1},
        {"VAL", 0},
        {"PREV", 2},
        {NULL, NONE},

};



typedef struct ESCAPE_CODE
{

        char CODE_CHAR;
        unsigned char VALUE;

}
ESCAPE_CODE;



ESCAPE_CODE ESCAPE_CODE_LIST[] =
{

        {'t', 9},
        {'n', 10},
        {'v', 11},
        {'a', 7},
        {'b', 8},
        {'r', 13},
        {'f', 12},
        {'\\', 92},
        {NONE, NONE}

};




size_t JSMC_CURRENT_SOC = 0;
int IS_END_OF_CODE = FALSE;
int IS_JSMC_ERROR = FALSE;
int MODE;





void GET__JSM_STATEMENT__IN__BYTECODE(char* JSMCODE, size_t STARTING_INDEX, size_t STATEMENT_LENGTH, char* BYTECODE_STATEMENT);


void APPEND__STATEMENT_TO_BYTECODE(char* BYTECODE, const size_t STARTING_INDEX, const char* STATEMENT);


int STRING_TO_UNSIGNED_LONG_LONG(const char* STRING, unsigned long long* UNSIGNED_LONG_LONG);


int TOKEN_EQUALS(const char* TOKEN, const size_t TOKEN_LENGTH, const char* COMPARE_TOKEN);


unsigned char GET_INSTRUCTION_VALUE(const char* TOKEN, const size_t TOKEN_LENGTH);


unsigned long GET_OPERAND_VALUE(const char* TOKEN, const size_t TOKEN_LENGTH, unsigned char* OPERAND_TYPE);


int TOKEN_IS_NUMBER(const char* TOKEN, const size_t TOKEN_LENGTH);


unsigned long long TOKEN_TO_NUMBER(const char* TOKEN, const size_t TOKEN_LENGTH);


void JSMC_LOG_COMPILE_ERROR(const char* MESSAGE);


void PARSE_JSMCODE_DATA_TO_BYTECODE(char* JSMCODE, const size_t JSMCODE_SIZE, char* BYTECODE, size_t* JSMCODE_INDEX, size_t* BYTECODE_INDEX);





int JSM__CHECK_BYTECODE_SIZE(char* JSMCODE, const size_t JSMCODE_SIZE, size_t* BYTECODE_SIZE)
{

        MODE = SIZE_CHECK_MODE;
        IS_JSMC_ERROR = FALSE;


        char BYTECODE_STATEMENT[BYTECODE_STATEMENT_SIZE];
        BYTECODE_STATEMENT[0] = 0;
        size_t JSMCODE_INDEX = 0;
        size_t BYTECODE_INDEX = 0;
        JSMC_CURRENT_SOC = 1;



        for (; JSMCODE_INDEX < JSMCODE_SIZE; JSMCODE_INDEX ++)
        {

                if (JSMCODE[JSMCODE_INDEX] == '\n')
                {

                        continue;

                }


                size_t STATEMENT_LENGTH = 0;
                size_t INDEX = JSMCODE_INDEX;


                // [FIND STATEMENT LENGTH]
                {

                        for (; JSMCODE[INDEX] != ';' && INDEX < JSMCODE_SIZE; )
                        {

                                if (JSMCODE[INDEX] == '/' && JSMCODE[INDEX + 1] == '/')
                                {

                                        for (; JSMCODE[INDEX] != '\n' && INDEX < JSMCODE_SIZE; INDEX ++, STATEMENT_LENGTH ++);

                                }
                                else
                                {

                                        INDEX ++;

                                        STATEMENT_LENGTH ++;

                                }

                        }

                }


                if (INDEX >= JSMCODE_SIZE)
                {

                        JSMCODE_INDEX += STATEMENT_LENGTH;


                        break;

                }


                // [COMPILE AND ADD STATEMENT]
                {

                        GET__JSM_STATEMENT__IN__BYTECODE(JSMCODE, JSMCODE_INDEX, STATEMENT_LENGTH, BYTECODE_STATEMENT);


                        if (IS_JSMC_ERROR)
                        {

                                printf("\n\033[1;31m[JSMC SIZE CHECK FAILED.]\033[0m\n\n");


                                return JSM_ERROR;

                        }

                }


                // [INCREMENT NUMBERS]
                {

                        JSMCODE_INDEX += STATEMENT_LENGTH;


                        JSMC_CURRENT_SOC ++;


                        BYTECODE_INDEX += BYTECODE_STATEMENT_SIZE;

                }


                if (BYTECODE_STATEMENT[0] == END)
                {

                        break;

                }

        }


        if (JSMCODE_INDEX >= JSMCODE_SIZE)
        {

                JSMC_LOG_COMPILE_ERROR("MISSING END; STATEMENT");


                printf("\n\033[1;31m[JSMC SIZE CHECK FAILED.]\033[0m\n\n");


                return JSM_ERROR;

        }


        PARSE_JSMCODE_DATA_TO_BYTECODE(JSMCODE, JSMCODE_SIZE, NULL, &JSMCODE_INDEX, &BYTECODE_INDEX);


        *BYTECODE_SIZE = BYTECODE_INDEX;


        return JSM_OK;

}


int JSM__COMPILE_TO_BYTECODE(char* JSMCODE, char* BYTECODE, const size_t JSMCODE_SIZE, size_t* BYTECODE_SIZE)
{

        MODE = COMPILE_MODE;
        IS_JSMC_ERROR = FALSE;
        char BYTECODE_STATEMENT[BYTECODE_STATEMENT_SIZE];
        BYTECODE_STATEMENT[0] = 0;
        size_t JSMCODE_INDEX = 0;
        size_t BYTECODE_INDEX = 0;
        JSMC_CURRENT_SOC = 1;



        for (; JSMCODE_INDEX < JSMCODE_SIZE; JSMCODE_INDEX ++)
        {

                if (JSMCODE[JSMCODE_INDEX] == '\n')
                {

                        continue;

                }


                size_t INDEX = JSMCODE_INDEX;
                size_t STATEMENT_LENGTH = 0;



                // [FIND STATEMENT LENGTH]
                {

                        for (; JSMCODE[INDEX] != ';' && INDEX < JSMCODE_SIZE; )
                        {

                                if (JSMCODE[INDEX] == '/' && JSMCODE[INDEX + 1] == '/')
                                {

                                        for (; JSMCODE[INDEX] != '\n' && INDEX < JSMCODE_SIZE; INDEX ++, STATEMENT_LENGTH ++);

                                }
                                else
                                {

                                        INDEX ++;

                                        STATEMENT_LENGTH ++;

                                }

                        }

                }


                if (INDEX >= JSMCODE_SIZE)
                {

                        JSMCODE_INDEX += STATEMENT_LENGTH;


                        break;

                }



                // [COMPILE AND ADD STATEMENT]
                {

                        GET__JSM_STATEMENT__IN__BYTECODE(JSMCODE, JSMCODE_INDEX, STATEMENT_LENGTH, BYTECODE_STATEMENT);


                        if (IS_JSMC_ERROR)
                        {

                                printf("\n\033[1;31m[JSMC COMPILATION FAILED.]\033[0m\n\n");


                                return JSM_ERROR;

                        }


                        APPEND__STATEMENT_TO_BYTECODE(BYTECODE, BYTECODE_INDEX, BYTECODE_STATEMENT);

                }


                // [INCREMENT NUMBERS]
                {

                        JSMCODE_INDEX += STATEMENT_LENGTH;


                        JSMC_CURRENT_SOC ++;


                        BYTECODE_INDEX += BYTECODE_STATEMENT_SIZE;

                }


                if (BYTECODE_STATEMENT[0] == END)
                {

                        break;

                }

        }


        if (JSMCODE_INDEX >= JSMCODE_SIZE)
        {

                JSMC_LOG_COMPILE_ERROR("MISSING END; STATEMENT");


                printf("\n\033[1;31m[JSMC COMPILATION FAILED.]\033[0m\n\n");


                return JSM_ERROR;

        }


        PARSE_JSMCODE_DATA_TO_BYTECODE(JSMCODE, JSMCODE_SIZE, BYTECODE, &JSMCODE_INDEX, &BYTECODE_INDEX);


        *BYTECODE_SIZE = BYTECODE_INDEX;


        return JSM_OK;

}


void PARSE_JSMCODE_DATA_TO_BYTECODE(char* JSMCODE, const size_t JSMCODE_SIZE, char* BYTECODE, size_t* JSMCODE_INDEX, size_t* BYTECODE_INDEX)
{

        for ((*JSMCODE_INDEX) ++; (*JSMCODE_INDEX) < JSMCODE_SIZE; (*JSMCODE_INDEX) ++)
        {

                char CHAR = JSMCODE[(*JSMCODE_INDEX)];


                if (CHAR == '\n' || CHAR == '\t')
                {

                        continue;

                }


                if (CHAR != '\\')
                {

                        SKIPPED:


                        if (MODE == COMPILE_MODE)
                        {

                                BYTECODE[(*BYTECODE_INDEX)] = CHAR;

                        }

                }
                else
                // [GET ESCAPE CODE]
                {

                        (*JSMCODE_INDEX) ++;



                        // [HOT CASE '\0']
                        if (JSMCODE[(*JSMCODE_INDEX)] == '0')
                        {

                                CHAR = 0;


                                goto SKIPPED;

                        }
                        // [HOT CASE '\n']
                        else if (JSMCODE[(*JSMCODE_INDEX)] == 'n')
                        {

                                CHAR = '\n';


                                goto SKIPPED;

                        }


                        // [FIND CHAR-ED ESCAPE CODE]
                        {

                                char ESCAPE_CODE_CHAR = JSMCODE[(*JSMCODE_INDEX)];


                                unsigned char INDEX = 0;


                                for (; ESCAPE_CODE_LIST[INDEX].CODE_CHAR != NONE; INDEX ++)
                                {

                                        if (ESCAPE_CODE_CHAR == ESCAPE_CODE_LIST[INDEX].CODE_CHAR)
                                        {

                                                CHAR = ESCAPE_CODE_LIST[INDEX].VALUE;


                                                break;

                                        }

                                }


                                if (ESCAPE_CODE_LIST[INDEX].CODE_CHAR != NONE)
                                {

                                        goto SKIPPED;

                                }

                        }


                        // [TRY AND GET NUMBERED ESCAPE CODE]
                        {

                                unsigned char NUMBERED_ESCAPE_CODE_LENGTH = 0;


                                // [GET NUMBER LENGTH]
                                {


                                        for (size_t INDEX = (*JSMCODE_INDEX); JSMCODE[INDEX] >= 48 && JSMCODE[INDEX] <= 57; INDEX ++)
                                        {

                                                NUMBERED_ESCAPE_CODE_LENGTH ++;


                                                if (JSMCODE[INDEX] == '0' && NUMBERED_ESCAPE_CODE_LENGTH == 1)
                                                {

                                                        break;

                                                }

                                        }

                                }


                                if (NUMBERED_ESCAPE_CODE_LENGTH == 0)
                                {

                                        JSMC_LOG_COMPILE_ERROR("UNKNOWN ESCAPE CODE");


                                        printf("└──────────────── '\\%c'\n", JSMCODE[(*JSMCODE_INDEX)]);


                                        return;

                                }


                                // [CONVERT TO NUMBER]
                                {

                                        unsigned long long CHAR_VALUE = TOKEN_TO_NUMBER(JSMCODE + (*JSMCODE_INDEX), NUMBERED_ESCAPE_CODE_LENGTH);


                                        if (CHAR_VALUE > 255)
                                        {

                                                // [TODO : THROW WRAP WARNING]

                                        }


                                        CHAR = CHAR_VALUE;

                                }


                                (*JSMCODE_INDEX) += NUMBERED_ESCAPE_CODE_LENGTH - 1;

                        }


                }


                if (MODE == COMPILE_MODE)
                {

                        BYTECODE[(*BYTECODE_INDEX)] = CHAR;

                }


                (*BYTECODE_INDEX) ++;

        }

}


void GET__JSM_STATEMENT__IN__BYTECODE(char* JSMCODE, size_t STARTING_INDEX, size_t STATEMENT_LENGTH, char* BYTECODE_STATEMENT)
{

        #define IS_SPACED_CHAR(INDEX) (JSMCODE[INDEX] == ' ' || JSMCODE[INDEX] == '\n' ||  JSMCODE[INDEX] == '\t')

        const size_t ENDING_LENGTH = STARTING_INDEX + STATEMENT_LENGTH;
        unsigned char TOKEN_COUNT = 0;
        unsigned char INSTRUCTION_INDEX = 0;


        for (unsigned char INDEX = 0; INDEX < BYTECODE_STATEMENT_SIZE; INDEX ++)
        {

                BYTECODE_STATEMENT[INDEX] = 0;

        }


        for (size_t JSMCODE_INDEX = STARTING_INDEX; JSMCODE_INDEX < ENDING_LENGTH; JSMCODE_INDEX ++)
        {

                if (TOKEN_COUNT >= 3)
                {

                        JSMC_LOG_COMPILE_ERROR("MISSING ';' OR MORE THAN 2 OPERANDS");


                        return;

                }


                if (IS_SPACED_CHAR(JSMCODE_INDEX) || JSMCODE[JSMCODE_INDEX] == '{'  || JSMCODE[JSMCODE_INDEX] == '}' )
                {

                        continue;

                }


                if (JSMCODE[JSMCODE_INDEX] == '/' && JSMCODE[JSMCODE_INDEX + 1] == '/')
                {

                        for (; JSMCODE[JSMCODE_INDEX] != '\n'; JSMCODE_INDEX ++);


                        continue;

                }


                char* TOKEN = JSMCODE + JSMCODE_INDEX;
                TOKEN_COUNT ++;
                size_t TOKEN_LENGTH = 0;



                // [SET TOKEN AND LENGTH]
                {

                        for (size_t TOKEN_INDEX = JSMCODE_INDEX; (!IS_SPACED_CHAR(TOKEN_INDEX) && TOKEN_INDEX < ENDING_LENGTH); TOKEN_INDEX ++)
                        {

                                TOKEN_LENGTH ++;

                        }

                }


                if (TOKEN_COUNT == 1)
                // [SET INSTRUCTION VALUE]
                {

                        INSTRUCTION_INDEX = GET_INSTRUCTION_VALUE(TOKEN, TOKEN_LENGTH);


                        BYTECODE_STATEMENT[0] = INSTRUCTION_INDEX;


                        if (IS_JSMC_ERROR)
                        {

                                return;

                        }

                }
                else if (TOKEN_COUNT > INSTRUCTION_STRINGS_LIST[INSTRUCTION_INDEX].REQUIRED_OPERANDS + 1)
                {

                        JSMC_LOG_COMPILE_ERROR("TOO MANY OPERANDS FOR INSTRUCTION");


                        printf("└──────────────── '%s' , EXPECTED : %d OPERAND(S) ONLY\n", INSTRUCTION_STRINGS_LIST[INSTRUCTION_INDEX].STRING, INSTRUCTION_STRINGS_LIST[INSTRUCTION_INDEX].REQUIRED_OPERANDS);


                        return;

                }
                else
                // [SET OPERAND VALUE]
                {

                        unsigned char OPERAND_TYPE;
                        unsigned long long OPERAND_VALUE = GET_OPERAND_VALUE(TOKEN, TOKEN_LENGTH, &OPERAND_TYPE);


                        if (IS_JSMC_ERROR)
                        {

                                return;

                        }


                        BYTECODE_STATEMENT[TOKEN_COUNT - 1] = OPERAND_TYPE;


                        char* OPERAND_VALUE_BYTES = (char*)&OPERAND_VALUE;
                        size_t BYTECODE_OPERAND_INDEX = (TOKEN_COUNT == 2) ? 8 : 16;


                        for (unsigned short INDEX = 0; INDEX < 8; INDEX ++)
                        {

                                BYTECODE_STATEMENT[BYTECODE_OPERAND_INDEX + INDEX] = OPERAND_VALUE_BYTES[INDEX];

                        }

                }


                JSMCODE_INDEX += TOKEN_LENGTH - 1;

        }


        if (TOKEN_COUNT >= 1 && TOKEN_COUNT - 1 < INSTRUCTION_STRINGS_LIST[INSTRUCTION_INDEX].REQUIRED_OPERANDS)
        {

                JSMC_LOG_COMPILE_ERROR("TOO FEW OPERANDS FOR INSTRUCTION");


                printf("└──────────────── '%s' , EXPECTED : %d OPERAND(S)\n", INSTRUCTION_STRINGS_LIST[INSTRUCTION_INDEX].STRING, INSTRUCTION_STRINGS_LIST[INSTRUCTION_INDEX].REQUIRED_OPERANDS);


                return;

        }

}


unsigned char GET_INSTRUCTION_VALUE(const char* TOKEN, const size_t TOKEN_LENGTH)
{

        unsigned char INDEX = 0;


        for (; INSTRUCTION_STRINGS_LIST[INDEX].STRING != NULL; INDEX ++)
        {

                if (TOKEN_EQUALS(TOKEN, TOKEN_LENGTH, INSTRUCTION_STRINGS_LIST[INDEX].STRING))
                {

                        return INDEX;

                }

        }


        if (INSTRUCTION_STRINGS_LIST[INDEX].STRING == NULL)
        {

                // [LOG ERROR]
                {

                        JSMC_LOG_COMPILE_ERROR("UNKNOWN INSRUCTION");


                        printf("└──────────────── '");


                        for (size_t INDEX = 0; INDEX < TOKEN_LENGTH; INDEX ++)
                        {

                                printf("%c", TOKEN[INDEX]);

                        }


                        printf("'\n");

                }

        }


        return 0;

}


unsigned long GET_OPERAND_VALUE(const char* TOKEN, const size_t TOKEN_LENGTH, unsigned char* OPERAND_TYPE)
{

        for (unsigned char INDEX = 0; REGISTER_STRINGS[INDEX] != NULL; INDEX ++)
        {

                if (TOKEN_EQUALS(TOKEN, TOKEN_LENGTH, REGISTER_STRINGS[INDEX]))
                {

                        *OPERAND_TYPE |= (REG << OPERAND_1_TYPE_INDEX);


                        return INDEX;

                }

        }

        for (unsigned char INDEX = 0; MACRO__STRING_TO_VALUE__LIST[INDEX].STRING != NULL; INDEX ++)
        {

                const MACRO__STRING_TO_VALUE CURRENT_MACRO__STRING_TO_VALUE = MACRO__STRING_TO_VALUE__LIST[INDEX];


                if (TOKEN_EQUALS(TOKEN, TOKEN_LENGTH, CURRENT_MACRO__STRING_TO_VALUE.STRING))
                {

                        *OPERAND_TYPE |= (REG << OPERAND_1_TYPE_INDEX);


                        return CURRENT_MACRO__STRING_TO_VALUE.VALUE;

                }

        }


        if (TOKEN_IS_NUMBER(TOKEN, TOKEN_LENGTH))
        {

                *OPERAND_TYPE = VAL;


                return TOKEN_TO_NUMBER(TOKEN, TOKEN_LENGTH);

        }


        // [LOG ERROR]
        {

                JSMC_LOG_COMPILE_ERROR("UNKNOWN OPERAND");


                printf("└──────────────── '");


                for (size_t INDEX = 0; INDEX < TOKEN_LENGTH; INDEX ++)
                {

                        printf("%c", TOKEN[INDEX]);

                }


                printf("'\n");

        }


        return 0;

}


int TOKEN_EQUALS(const char* TOKEN, const size_t TOKEN_LENGTH, const char* COMPARE_TOKEN)
{

        size_t INDEX;


        // [COMPARE LENGTH]
        {

                size_t COMPARE_TOKEN_LENGTH = 0;


                while (COMPARE_TOKEN[COMPARE_TOKEN_LENGTH] != '\0') COMPARE_TOKEN_LENGTH++;


                if (TOKEN_LENGTH != COMPARE_TOKEN_LENGTH)
                {

                        return FALSE;

                }

        }


        for (INDEX = 0; INDEX < TOKEN_LENGTH; INDEX ++)
        {

                if (TOKEN[INDEX] != COMPARE_TOKEN[INDEX])
                {

                        return FALSE;

                }

        }


        return TRUE;

}


int TOKEN_IS_NUMBER(const char* TOKEN, const size_t TOKEN_LENGTH)
{

        if (TOKEN[0] == '0' && TOKEN_LENGTH != 1)
        {

                return FALSE;

        }


        for (size_t INDEX = 0; INDEX < TOKEN_LENGTH; INDEX ++)
        {

                const char CHAR = TOKEN[INDEX];


                if (!(TOKEN[INDEX] >= 48 && CHAR <= 57))
                {

                        return FALSE;

                }

        }


        return TRUE;

}


unsigned long long TOKEN_TO_NUMBER(const char* TOKEN, const size_t TOKEN_LENGTH)
{

        unsigned long long NUMBER = 0;


        for (size_t INDEX = 0; INDEX < TOKEN_LENGTH; INDEX ++)
        {

                NUMBER = (NUMBER * 10) + (TOKEN[INDEX] - '0');

        }


        return NUMBER;

}


void APPEND__STATEMENT_TO_BYTECODE(char* BYTECODE, const size_t STARTING_INDEX, const char* STATEMENT)
{

        for (size_t INDEX = 0; INDEX < BYTECODE_STATEMENT_SIZE; INDEX ++)
        {

                BYTECODE[INDEX + STARTING_INDEX] = STATEMENT[INDEX];

        }

}


void JSMC_LOG_COMPILE_ERROR(const char* MESSAGE)
{

        if (MODE == COMPILE_MODE)
        {

                fprintf(stderr, "\n\033[1;31m[JSMC COMPILE ERROR]\033[0m : | STATEMENT %lu : %s |\n", JSMC_CURRENT_SOC, MESSAGE);

        }
        else if (MODE == SIZE_CHECK_MODE)
        {

                fprintf(stderr, "\n\033[1;31m[JSMC SIZE CHECK ERROR]\033[0m : | STATEMENT %lu : %s |\n", JSMC_CURRENT_SOC, MESSAGE);

        }


        IS_JSMC_ERROR = TRUE;

}
