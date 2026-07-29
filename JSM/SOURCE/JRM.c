#include "../JSM.h"




#define CAST_OPERAND_TO_TYPE(OPERAND, NUMBER) OPERAND = (BYTECODE_STATEMENT[NUMBER]) ? REGISTER_LIST[OPERAND] : OPERAND
#define CHAR_PTR_CAST(VALUE) ((char*)&VALUE)



#define EXIT_WITH_END_WARNING_CODE 3



// [GLOBAL VARIABLES]
//
        unsigned long REGISTER_LIST[REGISTER_COUNT];

        char* MEMORY_SPACE;

        char* BYTECODE_STATEMENT;

        size_t JSM_CURRENT_SOC;

        long IS_RUNTIME_ERROR;

        long EXIT_REQUESTED;

        long EXIT_CODE;

        long DO_NOT_INCREMENT_STATMENT_INDEX;

        size_t CODE_INDEX;

        unsigned long INSTRUCTION;
        unsigned long OPERAND_1;
        unsigned long OPERAND_2;

//


// [INSTRUCTIONS DECLERATION]
//
        void EXIT_INSTRUCTION();

        void RETURN_INSTRUCTION();

        void END_INSTRUCTION();

        void JUMP_INSTRUCTION();

        void SKIP_INSTRUCTION();

        void SET_INSTRUCTION();

        void SETMODE_INSTRUCTION();

        void ADD_INSTRUCTION();

        void SUB_INSTRUCTION();

        void MUL_INSTRUCTION();

        void DIV_INSTRUCTION();

        void CMPE_INSTRUCTION();

        void CMPH_INSTRUCTION();

        void CMPL_INSTRUCTION();

        void CMPHE_INSTRUCTION();

        void CMPLE_INSTRUCTION();

        void PUSH_INSTRUCTION();

        void POP_INSTRUCTION();

        void LOAD_INSTRUCTION();

//


void (*INSTRUCTION_LIST[])(void) =
{

        EXIT_INSTRUCTION,
        RETURN_INSTRUCTION,
        END_INSTRUCTION,
        JUMP_INSTRUCTION,
        SKIP_INSTRUCTION,
        SETMODE_INSTRUCTION,
        SET_INSTRUCTION,
        ADD_INSTRUCTION,
        SUB_INSTRUCTION,
        MUL_INSTRUCTION,
        DIV_INSTRUCTION,
        CMPE_INSTRUCTION,
        CMPH_INSTRUCTION,
        CMPL_INSTRUCTION,
        CMPHE_INSTRUCTION,
        CMPLE_INSTRUCTION,
        PUSH_INSTRUCTION,
        POP_INSTRUCTION,
        LOAD_INSTRUCTION,

};




void JRM_LOG_ERROR(const char* MESSAGE);

void LOG_PRELOAD_ERROR(const char* MESSAGE);

int JRM__INIT(const char* CODE, const size_t CODE_SIZE, const size_t STACK_SIZE_MB);

void JSM__EXIT();




int JSM__READ_FILE_TO_JSMCODE(const char* FILE_PATH, size_t* JSMCODE_LENGTH, char** JSMCODE)
{

        FILE* JSM_FILE = fopen(FILE_PATH, "rb");



        if (JSM_FILE == NULL)
        {

                JRM_LOG_ERROR("FAILED TO OPEN .jsm FILE");


                return JSM_ERROR;
        }


        fseek(JSM_FILE, 0, SEEK_END);


        *JSMCODE_LENGTH = ftell(JSM_FILE);


        if ((*JSMCODE_LENGTH) < 0)
        {

                JRM_LOG_ERROR("FAILED TO OPEN .jsm FILE");


                fclose(JSM_FILE);


                return JSM_ERROR;
        }



        rewind(JSM_FILE);


        (*JSMCODE) = (char *)malloc((*JSMCODE_LENGTH) + 1);



        if ((*JSMCODE) == NULL)
        {

                JRM_LOG_ERROR("FAILED TO OPEN .jsm FILE");


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


int JRM__INIT(const char* CODE, const size_t BYTECODE_SIZE, const size_t STACK_SIZE_MB)
{

        // [SETUP MEMORY SPACE]
        {

                const size_t MEMORY_SPACE_SIZE = BYTECODE_SIZE + (1024 * 1024 * STACK_SIZE_MB);


                MEMORY_SPACE = malloc(MEMORY_SPACE_SIZE);


                if (MEMORY_SPACE == NULL)
                {

                        LOG_PRELOAD_ERROR("FAILED TO ALLOCATE PROGRAM MEMORY SPACE");


                        return JSM_ERROR;

                }

        }


        // [COPY CODE INTO MEMORY SPACE]
        {

                for (size_t INDEX = 0; INDEX < BYTECODE_SIZE; INDEX ++)
                {

                        MEMORY_SPACE[INDEX] = CODE[INDEX];

                }

        }


        // [SETUP REGISTERS]
        {

                for (unsigned char INDEX = 0; INDEX < REGISTER_COUNT; INDEX ++)
                {

                        REGISTER_LIST[INDEX] = 0;

                }

        }


        // [SET RDP & RDB]
        {

                size_t INDEX;


                // [FIND END INSTRUCTION IN CODE]
                // ------------------
                for (INDEX = 0; MEMORY_SPACE[INDEX] != END; INDEX += 19);


                INDEX += 19;


                REGISTER_LIST[RDP] = INDEX;
                REGISTER_LIST[RDB] = INDEX;

        }


        // [SET RSP & RSB]
        {

                REGISTER_LIST[RSP] = BYTECODE_SIZE;
                REGISTER_LIST[RSB] = BYTECODE_SIZE;

        }


        // [SET / RESET RUNTIME DATA]
        {

                IS_RUNTIME_ERROR = FALSE;
                EXIT_REQUESTED = FALSE;

                CODE_INDEX = 0;
                JSM_CURRENT_SOC = 1;

        }


        return JSM_OK;

}


int JRM__RUN(const char* CODE, const size_t CODE_SIZE, const size_t STACK_SIZE_MB)
{

        const int INIT_STATUS = JRM__INIT(CODE, CODE_SIZE, STACK_SIZE_MB);


        if (INIT_STATUS == JSM_ERROR)
        {

                return JSM_ERROR;

        }


        while (TRUE)
        {

                BYTECODE_STATEMENT = (char*)(CODE + CODE_INDEX);


                // [EXECUTE INSTRUCTION]
                {

                        INSTRUCTION = CODE[CODE_INDEX];


                        // [WRITE BOTH OPERANDS]
                        {

                                OPERAND_1 = *((unsigned long*)&(CODE[CODE_INDEX + 3]));
                                OPERAND_2 = *((unsigned long*)&(CODE[CODE_INDEX + 11]));

                        }


                        DO_NOT_INCREMENT_STATMENT_INDEX = FALSE;


                        INSTRUCTION_LIST[INSTRUCTION]();

                }


                if (EXIT_REQUESTED)
                {

                        JSM__EXIT();


                        return JSM_OK;

                }


                // [INCREMENT]
                {

                        CODE_INDEX += (!DO_NOT_INCREMENT_STATMENT_INDEX) * 19;


                        JSM_CURRENT_SOC ++;

                }

        }


        return JSM_OK;

}


void JSM__EXIT()
{

        free(MEMORY_SPACE);



        // [PRINT EXIT STATUS]
        {

                printf("\033[1;31m[JRM EXIT]\033[0m : ");


                if (EXIT_CODE == 0)
                {

                        printf("PROGRAM SUCCESSFULLY EXITED WITH CODE 0");

                }
                else if (EXIT_CODE == 3)
                {

                        printf("PROGRAM EXITED WITH CODE 3 : END STATMENT WAS CALLED, MAKE SURE TO CALL EXIT BEFORE END");

                }
                else if (EXIT_CODE == 11)
                {

                        printf("PROGRAM EXITED WITH CODE 11 : SEGMENTATION FAULT");

                }
                else
                {

                        printf("PROGRAM EXITED WITH CODE %ld", EXIT_CODE);

                }


                printf("\n");

        }

}


void EXIT_INSTRUCTION()
{

        EXIT_REQUESTED = TRUE;
        EXIT_CODE = OPERAND_1;

}


void CMPE_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_1, 1);
        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;


        CODE_INDEX += 19 + (19 * (!(OPERAND_1 == OPERAND_2)));

}


void CMPH_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_1, 1);
        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;


        CODE_INDEX += 19 + (19 * (!(OPERAND_1 > OPERAND_2)));

}


void CMPL_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_1, 1);
        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;


        CODE_INDEX += 19 + (19 * (!(OPERAND_1 < OPERAND_2)));

}


void CMPHE_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_1, 1);
        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;


        CODE_INDEX += 19 + (19 * (!(OPERAND_1 >= OPERAND_2)));

}


void CMPLE_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_1, 1);
        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;


        CODE_INDEX += 19 + (19 * (!(OPERAND_1 <= OPERAND_2)));

}


void RETURN_INSTRUCTION()
{

        CODE_INDEX =  REGISTER_LIST[RRA] * 19;


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;

}


void SKIP_INSTRUCTION()
{



}


void END_INSTRUCTION()
{

        EXIT_REQUESTED = TRUE;
        EXIT_CODE = 3;

}


void SET_INSTRUCTION()
{

        REGISTER_LIST[OPERAND_1] = OPERAND_2;

}


void JUMP_INSTRUCTION()
{

        if (OPERAND_1 == 0)
        {

                JRM_LOG_ERROR("SOC OF JUMP STATEMENT MUST NOT BE ZERO");


                EXIT_REQUESTED = TRUE;
                EXIT_CODE = 1;

        }


        CODE_INDEX = (OPERAND_1 - 1) * 19;


        DO_NOT_INCREMENT_STATMENT_INDEX = TRUE;

}


void PUSH_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_1, 1);


        for (unsigned char INDEX = 0; INDEX < OPERAND_2; INDEX ++)
        {

                MEMORY_SPACE[INDEX + REGISTER_LIST[RSP]] = CHAR_PTR_CAST(OPERAND_1)[INDEX];

        }


        REGISTER_LIST[RSP] += OPERAND_2;

}


void POP_INSTRUCTION()
{

        REGISTER_LIST[RSP] -= OPERAND_2;


        for (unsigned char INDEX = 0; INDEX < OPERAND_2; INDEX ++)
        {

                CHAR_PTR_CAST(REGISTER_LIST[OPERAND_1])[INDEX] = MEMORY_SPACE[INDEX + REGISTER_LIST[RSP]];

        }

}


void LOAD_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        for (unsigned char INDEX = 0; INDEX < OPERAND_2; INDEX ++)
        {

                CHAR_PTR_CAST(REGISTER_LIST[OPERAND_1])[INDEX] = MEMORY_SPACE[INDEX + REGISTER_LIST[RLA]];

        }

}


void SETMODE_INSTRUCTION()
{

        MEMORY_SPACE[CODE_INDEX + 20] = OPERAND_1;
        MEMORY_SPACE[CODE_INDEX + 21] = OPERAND_2;

}


void ADD_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        REGISTER_LIST[OPERAND_1] += OPERAND_2;

}


void SUB_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        REGISTER_LIST[OPERAND_1] -= OPERAND_2;

}


void MUL_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        REGISTER_LIST[OPERAND_1] *= OPERAND_2;

}


void DIV_INSTRUCTION()
{

        CAST_OPERAND_TO_TYPE(OPERAND_2, 2);


        REGISTER_LIST[OPERAND_1] /= OPERAND_2;

}


void JRM_LOG_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JRM ERROR]\033[0m : STATEMENT : %lu | %s\n", JSM_CURRENT_SOC, MESSAGE);


        IS_RUNTIME_ERROR = TRUE;

}


void LOG_PRELOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : | %s |\n", MESSAGE);


        IS_RUNTIME_ERROR = TRUE;

}
