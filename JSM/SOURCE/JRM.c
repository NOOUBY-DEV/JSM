#include "../JSM.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>



#define CAST_OPERAND_TO_TYPE(OPERAND, NUMBER) (*(JRM.MEMORY_SPACE + JRM.CODE_INDEX + NUMBER)) ? JRM.REGISTER_LIST[OPERAND] : OPERAND
#define CHAR_PTR_CAST(VALUE) ((char*)&VALUE)



#define EXIT_WITH_END_WARNING_CODE 3



typedef struct JRM_DATA
{

        unsigned long REGISTER_LIST[REGISTER_COUNT];

        char* restrict MEMORY_SPACE;

        size_t CODE_INDEX;
        size_t JSM_CURRENT_SOC;

        size_t MEMORY_SPACE_SIZE;

        size_t TOTAL_SOC;

        unsigned long OPERAND_1;
        unsigned long OPERAND_2;

        unsigned long EXIT_CODE;

        unsigned long EXIT_REQUESTED;
        unsigned long INCREMENT_STATMENT_INDEX;

}
JRM_DATA;



JRM_DATA JRM;


// [INSTRUCTIONS DECLERATION]
//
        void EXIT_INSTRUCTION();

        void RETURN_INSTRUCTION();

        void END_INSTRUCTION();

        void JUMP_INSTRUCTION();

        void SKIP_INSTRUCTION();

        void SET_INSTRUCTION();

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


static void (*INSTRUCTION_LIST[])(void) =
{

        EXIT_INSTRUCTION,
        RETURN_INSTRUCTION,
        END_INSTRUCTION,
        JUMP_INSTRUCTION,
        SKIP_INSTRUCTION,
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

                JRM.MEMORY_SPACE_SIZE = BYTECODE_SIZE + (1024 * 1024 * STACK_SIZE_MB);


                JRM.MEMORY_SPACE = malloc(JRM.MEMORY_SPACE_SIZE);


                if (JRM.MEMORY_SPACE == NULL)
                {

                        LOG_PRELOAD_ERROR("FAILED TO ALLOCATE PROGRAM MEMORY SPACE");


                        return JSM_ERROR;

                }

        }


        // [COPY CODE INTO MEMORY SPACE]
        {

                for (size_t INDEX = 0; INDEX < BYTECODE_SIZE; INDEX ++)
                {

                        JRM.MEMORY_SPACE[INDEX] = CODE[INDEX];

                }

        }


        // [SETUP REGISTERS]
        {

                for (unsigned long INDEX = 0; INDEX < REGISTER_COUNT; INDEX ++)
                {

                        JRM.REGISTER_LIST[INDEX] = 0;

                }

        }


        // [SET RDP & RDB]
        {

                size_t INDEX;


                JRM.TOTAL_SOC = 0;


                // [FIND END INSTRUCTION IN CODE]
                // ------------------
                for (INDEX = 0; JRM.MEMORY_SPACE[INDEX] != END; INDEX += BYTECODE_STATEMENT_SIZE);


                INDEX += BYTECODE_STATEMENT_SIZE;


                JRM.TOTAL_SOC = INDEX / BYTECODE_STATEMENT_SIZE;


                JRM.REGISTER_LIST[RDP] = INDEX;
                JRM.REGISTER_LIST[RDB] = INDEX;

        }


        // [SET RSP & RSB]
        {

                JRM.REGISTER_LIST[RSP] = BYTECODE_SIZE;
                JRM.REGISTER_LIST[RSB] = BYTECODE_SIZE;

        }


        // [SET / RESET RUNTIME DATA]
        {

                JRM.EXIT_REQUESTED = FALSE;
                JRM.INCREMENT_STATMENT_INDEX = TRUE;

                JRM.CODE_INDEX = 0;
                JRM.JSM_CURRENT_SOC = 1;

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


        while (!JRM.EXIT_REQUESTED)
        {

                // [EXECUTE INSTRUCTION]
                {

                        const unsigned long INSTRUCTION = JRM.MEMORY_SPACE[JRM.CODE_INDEX];


                        // [WRITE BOTH OPERANDS]
                        {

                                JRM.OPERAND_1 = *((const unsigned long*)(JRM.MEMORY_SPACE + JRM.CODE_INDEX + 8));
                                JRM.OPERAND_2 = *((const unsigned long*)(JRM.MEMORY_SPACE + JRM.CODE_INDEX + 16));

                        }


                        INSTRUCTION_LIST[INSTRUCTION]();

                }


                // [INCREMENT]
                {

                        JRM.CODE_INDEX += JRM.INCREMENT_STATMENT_INDEX * BYTECODE_STATEMENT_SIZE;


                        #if defined (DEBUG)

                                JRM.JSM_CURRENT_SOC ++;

                        #endif


                        JRM.INCREMENT_STATMENT_INDEX = TRUE;

                }

        }


        JSM__EXIT();


        return JSM_OK;

}


void JSM__EXIT()
{

        free(JRM.MEMORY_SPACE);



        // [PRINT EXIT STATUS]
        {

                printf("\033[1;31m[JRM EXIT]\033[0m : ");


                if (JRM.EXIT_CODE == 0)
                {

                        printf("PROGRAM SUCCESSFULLY EXITED WITH CODE 0");

                }
                else if (JRM.EXIT_CODE == 3)
                {

                        printf("PROGRAM EXITED WITH CODE 3 : END STATMENT WAS CALLED, MAKE SURE TO CALL EXIT BEFORE END");

                }
                else if (JRM.EXIT_CODE == 4)
                {

                        printf("PROGRAM EXITED WITH CODE 4 : STATMENT JUMP OUT OF BOUNDS");

                }
                else if (JRM.EXIT_CODE == 11)
                {

                        printf("PROGRAM EXITED WITH CODE 11 : SEGMENTATION FAULT");

                }
                else
                {

                        printf("PROGRAM EXITED WITH CODE %ld", JRM.EXIT_CODE);

                }


                printf("\n");

        }

}


void EXIT_INSTRUCTION()
{

        JRM.EXIT_REQUESTED = TRUE;
        JRM.EXIT_CODE = JRM.OPERAND_1;

}


void CMPE_INSTRUCTION()
{

        const unsigned long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.INCREMENT_STATMENT_INDEX = FALSE;


        JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 != OPERAND_2));

}


void CMPH_INSTRUCTION()
{

        const unsigned long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.INCREMENT_STATMENT_INDEX = FALSE;


        JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 <= OPERAND_2));

}


void CMPL_INSTRUCTION()
{

        const unsigned long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.INCREMENT_STATMENT_INDEX = FALSE;


        JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 >= OPERAND_2));

}


void CMPHE_INSTRUCTION()
{

        const unsigned long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.INCREMENT_STATMENT_INDEX = FALSE;


        JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 < OPERAND_2));

}


void CMPLE_INSTRUCTION()
{

        const unsigned long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.INCREMENT_STATMENT_INDEX = FALSE;


        JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 > OPERAND_2));

}


void RETURN_INSTRUCTION()
{

        JRM.CODE_INDEX = JRM.REGISTER_LIST[RRA] * BYTECODE_STATEMENT_SIZE;


        JRM.INCREMENT_STATMENT_INDEX = FALSE;

}


void SKIP_INSTRUCTION()
{



}


void END_INSTRUCTION()
{

        JRM.EXIT_REQUESTED = TRUE;
        JRM.EXIT_CODE = 3;

}


void SET_INSTRUCTION()
{

        JRM.REGISTER_LIST[JRM.OPERAND_1] = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void JUMP_INSTRUCTION()
{

        const unsigned long STATEMENT_NUMBER = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);;


        if (STATEMENT_NUMBER == 0 || STATEMENT_NUMBER > JRM.TOTAL_SOC)
        {

                JRM.EXIT_CODE = 4;
                JRM.EXIT_REQUESTED = TRUE;


                return;

        }


        JRM.CODE_INDEX = (STATEMENT_NUMBER - 1) * BYTECODE_STATEMENT_SIZE;


        JRM.INCREMENT_STATMENT_INDEX = FALSE;

}


void PUSH_INSTRUCTION()
{

        const unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long PUSH_SIZE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);
        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



        if (CURRENT_RSP + PUSH_SIZE >= JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.EXIT_REQUESTED = TRUE;


                return;

        }


        #if !defined (RAW_JRM)

                memcpy(JRM.MEMORY_SPACE + CURRENT_RSP, &VALUE, PUSH_SIZE);

        #else

                for (unsigned char INDEX = 0; INDEX < JRM.OPERAND_2; INDEX ++)
                {

                        JRM.MEMORY_SPACE[INDEX + JRM.REGISTER_LIST[RSP]] = CHAR_PTR_CAST(JRM.OPERAND_1)[INDEX];

                }

        #endif



        JRM.REGISTER_LIST[RSP] += PUSH_SIZE;

}


void POP_INSTRUCTION()
{

        unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long POP_SIZE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP + 1 < POP_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.EXIT_REQUESTED = TRUE;


                return;

        }


        CURRENT_RSP -= POP_SIZE;


        #if !defined (RAW_JRM)

                memcpy(JRM.REGISTER_LIST + JRM.OPERAND_1, JRM.MEMORY_SPACE + CURRENT_RSP, POP_SIZE);

        #else

                for (unsigned char INDEX = 0; INDEX < JRM.OPERAND_2; INDEX ++)
                {

                        CHAR_PTR_CAST(JRM.REGISTER_LIST[JRM.OPERAND_1])[INDEX] = JRM.MEMORY_SPACE[INDEX + NEW_RSP];

                }

        #endif


        JRM.REGISTER_LIST[RSP] -= POP_SIZE;

}


void LOAD_INSTRUCTION()
{

        const unsigned long LOAD_SIZE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        // !========================== [NOT DONE] ==========================!
        for (unsigned char INDEX = 0; INDEX < JRM.OPERAND_2; INDEX ++)
        {

                CHAR_PTR_CAST(JRM.REGISTER_LIST[JRM.OPERAND_1])[INDEX] = JRM.MEMORY_SPACE[INDEX + JRM.REGISTER_LIST[RLA]];

        }

}


void ADD_INSTRUCTION()
{

        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.REGISTER_LIST[JRM.OPERAND_1] += VALUE;

}


void SUB_INSTRUCTION()
{

        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.REGISTER_LIST[JRM.OPERAND_1] -= VALUE;

}


void MUL_INSTRUCTION()
{

        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.REGISTER_LIST[JRM.OPERAND_1] *= VALUE;

}


void DIV_INSTRUCTION()
{

        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        JRM.REGISTER_LIST[JRM.OPERAND_1] /= VALUE;

}


void JRM_LOG_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JRM ERROR]\033[0m : STATEMENT : %lu | %s\n", JRM.JSM_CURRENT_SOC, MESSAGE);

}


void LOG_PRELOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : | %s |\n", MESSAGE);

}
