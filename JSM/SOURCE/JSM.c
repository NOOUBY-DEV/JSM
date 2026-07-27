#include "../JSM.h"




#define CHAR_PTR_CAST(VALUE) ((char*)&VALUE)

#define EXIT_WITH_END_WARNING_CODE 32



unsigned long REGISTER_LIST[REGISTER_COUNT];

char* STACK;

char* DATA;

size_t JSM_CURRENT_SOC = 1;

int IS_RUNTIME_ERROR = FALSE;

int DO_NOT_INCREMENT_STATMENT_INDEX = FALSE;


size_t CODE_INDEX;
unsigned char INSTRUCTION;
unsigned long OPERAND_1;
unsigned long OPERAND_2;
unsigned char OPERAND_1__TYPE;
unsigned char OPERAND_2__TYPE;



void EXIT_INSTRUCTION();

void RETURN_INSTRUCTION();

void END_INSTRUCTION();

void JUMP_INSTRUCTION();

void SKIP_INSTRUCTION();

void SET_INSTRUCTION();

void SETMODE_INSTRUCTION();

void MATH_INSTRUCTION();

void CMP_INTRUCTION();

void PUSH_INSTRUCTION();

void POP_INSTRUCTION();

void LOADMODE_INSTRUCTION();

void LOAD_INSTRUCTION();




void CAST_OPERAND_TO_TYPE(unsigned long* OPERAND);

void JSM_LOG_ERROR(const char* MESSAGE);

void LOG_PRELOAD_ERROR(const char* MESSAGE);



typedef void (*INSTRUCTION_FUNCTIONS)(void);



INSTRUCTION_FUNCTIONS INSTRUCTION_LIST[] =
{

        EXIT_INSTRUCTION,
        RETURN_INSTRUCTION,
        END_INSTRUCTION,
        JUMP_INSTRUCTION,
        SKIP_INSTRUCTION,
        SETMODE_INSTRUCTION,
        SET_INSTRUCTION,
        MATH_INSTRUCTION,
        MATH_INSTRUCTION,
        MATH_INSTRUCTION,
        MATH_INSTRUCTION,
        CMP_INTRUCTION,
        CMP_INTRUCTION,
        CMP_INTRUCTION,
        CMP_INTRUCTION,
        CMP_INTRUCTION,
        PUSH_INSTRUCTION,
        POP_INSTRUCTION,
        LOADMODE_INSTRUCTION,
        LOAD_INSTRUCTION,

};





int JSM__READ_FILE_TO_JSMCODE(const char* FILE_PATH, size_t* JSMCODE_LENGTH, char** JSMCODE)
{

        FILE* JSM_FILE = fopen(FILE_PATH, "rb");



        if (JSM_FILE == NULL)
        {

                JSM_LOG_ERROR("FAILED TO OPEN .jsm FILE");


                return JSM_ERROR;
        }


        fseek(JSM_FILE, 0, SEEK_END);


        *JSMCODE_LENGTH = ftell(JSM_FILE);


        if ((*JSMCODE_LENGTH) < 0)
        {

                JSM_LOG_ERROR("FAILED TO OPEN .jsm FILE");


                fclose(JSM_FILE);


                return JSM_ERROR;
        }



        rewind(JSM_FILE);


        (*JSMCODE) = (char *)malloc((*JSMCODE_LENGTH) + 1);



        if ((*JSMCODE) == NULL)
        {

                JSM_LOG_ERROR("FAILED TO OPEN .jsm FILE");


                fclose(JSM_FILE);


                return JSM_ERROR;
        }


        size_t bytes_read = fread((*JSMCODE), 1, (*JSMCODE_LENGTH), JSM_FILE);


        if ((*JSMCODE)[*JSMCODE_LENGTH - 1] == '\n')
        {

                (*JSMCODE_LENGTH) --;

        }



        fclose(JSM_FILE);



        return JSM_OK;

}



int JSM_INIT(const char* CODE, const size_t STACK_SIZE_MB)
{

        // [SETUP STACK]
        {

                STACK = malloc(1024 * 1024 * STACK_SIZE_MB);


                if (STACK == NULL)
                {

                        LOG_PRELOAD_ERROR("FAILED TO ALLOCATE MEMORY FOR STACK");


                        return JSM_ERROR;

                }


                memset(STACK, 0, 1024 * 1024 * STACK_SIZE_MB);

        }


        // [SETUP REGISTERS]
        {

                for (unsigned char INDEX = 0; INDEX < REGISTER_COUNT; INDEX ++)
                {

                        REGISTER_LIST[INDEX] = 0;

                }

        }


        // [SET RDP]
        {

                for (CODE_INDEX = 0; CODE[CODE_INDEX] != END; CODE_INDEX += 17);


                CODE_INDEX += 17;


                DATA = (char*)CODE + CODE_INDEX;

        }


        OPERAND_1__TYPE = REG;
        OPERAND_2__TYPE = VAL;


        return JSM_OK;

}


int JSM__RUN(const char* CODE, const size_t STACK_SIZE_MB)
{

        const int INIT_STATUS = JSM_INIT(CODE, STACK_SIZE_MB);


        printf("INITIALIZED !\n");


        if (INIT_STATUS == JSM_ERROR)
        {

                return JSM_ERROR;

        }


        for (CODE_INDEX = 0; CODE[CODE_INDEX] != END;)
        {

                INSTRUCTION = CODE[CODE_INDEX];


                // [WRITE BOTH OPERANDS]
                {

                        unsigned char USIGNED_LONG_SIZE = sizeof(unsigned long);


                        OPERAND_1 = *((unsigned long*)&(CODE[CODE_INDEX + 1]));
                        OPERAND_2 = *((unsigned long*)&(CODE[CODE_INDEX + 9]));

                        // memcpy(&OPERAND_1, CODE + CODE_INDEX + 1, sizeof(unsigned long));
                        // memcpy(&OPERAND_2, CODE + CODE_INDEX + 9, sizeof(unsigned long));

                }


                printf("SIZEOF ULONG : %lu  |  O1 : %lu  |  O2  :  %lu \n", sizeof(unsigned long), OPERAND_1, OPERAND_2);


                DO_NOT_INCREMENT_STATMENT_INDEX = FALSE;


                INSTRUCTION_LIST[INSTRUCTION]();


                if (!DO_NOT_INCREMENT_STATMENT_INDEX)
                {

                        CODE_INDEX += 17;

                }

        }



        return JSM_OK;

}


void JSM_EXIT(unsigned char EXIT_CODE)
{



}

void EXIT_INSTRUCTION()
{

        JSM_EXIT(OPERAND_1);

}


void CMP_INTRUCTION()
{

        char CONDITION;


        CAST_OPERAND_TO_TYPE(&OPERAND_1);
        CAST_OPERAND_TO_TYPE(&OPERAND_2);


        switch (INSTRUCTION)
        {

                case CMPE: CONDITION = (OPERAND_1 == OPERAND_2); break;
                case CMPH: CONDITION = (OPERAND_1 > OPERAND_2); break;
                case CMPL: CONDITION = (OPERAND_1 < OPERAND_2); break;
                case CMPHE: CONDITION = (OPERAND_1 >= OPERAND_2); break;
                case CMPLE: CONDITION = (OPERAND_1 <= OPERAND_2); break;

        }


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;


        CODE_INDEX += 17;


        if (!CONDITION)
        {

                CODE_INDEX += 17;

        }

}


void RETURN_INSTRUCTION()
{

        CODE_INDEX =  REGISTER_LIST[RRA] * 17;


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;

}


void SKIP_INSTRUCTION()
{



}


void END_INSTRUCTION()
{

        JSM_EXIT(EXIT_WITH_END_WARNING_CODE);

}


void SET_INSTRUCTION()
{

        REGISTER_LIST[OPERAND_1] = OPERAND_2;

}


void JUMP_INSTRUCTION()
{

        CODE_INDEX = OPERAND_1 * 17;


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;

}


void PUSH_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(&OPERAND_1);


        for (unsigned char INDEX = 0; INDEX < OPERAND_2; INDEX ++)
        {

                STACK[INDEX + REGISTER_LIST[RSP]] = CHAR_PTR_CAST(OPERAND_1)[INDEX];

        }


        REGISTER_LIST[RSP] += OPERAND_2;

}


void POP_INSTRUCTION()
{

        REGISTER_LIST[RSP] -= OPERAND_2;


        for (unsigned char INDEX = 0; INDEX < OPERAND_2; INDEX ++)
        {

                CHAR_PTR_CAST(REGISTER_LIST[OPERAND_1])[INDEX] = STACK[INDEX + REGISTER_LIST[RSP]];

        }

}


void LOADMODE_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(&OPERAND_2);


        REGISTER_LIST[RLA] = OPERAND_1;
        REGISTER_LIST[RLP] = OPERAND_2;

}


void LOAD_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(&OPERAND_2);


        char* AREA;


        if (REGISTER_LIST[RLA] == 0)
        {

                AREA = STACK;

        }
        else
        {

                AREA = DATA;

        }


        for (unsigned char INDEX = 0; INDEX < OPERAND_2; INDEX ++)
        {

                CHAR_PTR_CAST(REGISTER_LIST[OPERAND_1])[INDEX] = AREA[INDEX + REGISTER_LIST[RLP]];

        }

}


void SETMODE_INSTRUCTION()
{

        OPERAND_1__TYPE = OPERAND_1;
        OPERAND_2__TYPE = OPERAND_2;


        printf("SETMODE OK!\n");

}


void MATH_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(&OPERAND_2);


        unsigned long* REGISTER = &REGISTER_LIST[OPERAND_1];


        switch (INSTRUCTION)
        {

                case ADD: *REGISTER += OPERAND_2; break;

                case SUB: *REGISTER -= OPERAND_2; break;

                case MUL: *REGISTER *= OPERAND_2; break;

                case DIV: *REGISTER /= OPERAND_2; break;

        }

}


void CAST_OPERAND_TO_TYPE(unsigned long* OPERAND)
{

        unsigned char OPERAND_TYPE;


        if (OPERAND == &OPERAND_1)
        {

                OPERAND_TYPE = OPERAND_1__TYPE;

        }
        else
        {

                OPERAND_TYPE = OPERAND_2__TYPE;

        }


        if (OPERAND_TYPE == REG)
        {

                *OPERAND = REGISTER_LIST[*OPERAND];

        }

}


void JSM_LOG_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : |  %s |\n", MESSAGE);


        IS_RUNTIME_ERROR = TRUE;

}


void LOG_PRELOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : | %s |\n", MESSAGE);


        IS_RUNTIME_ERROR = TRUE;

}
