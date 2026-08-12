#include "../JSM.h"
#include <stddef.h>
#include <stdlib.h>




#if !defined (UNFETTERED)

        #define IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER) \
        if (REGISTER >= REGISTER_COUNT)\
        {\
                \
                JRM.EXIT_CODE = 5;\
                JRM.NOT_EXIT_REQUESTED = FALSE;\
        \
        \
                return;\
        \
        }

#else

        #define IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER)

#endif



#define CAST_OPERAND_TO_TYPE(OPERAND, NUMBER) (*(JRM.MEMORY_SPACE + JRM.CODE_INDEX + NUMBER)) ? JRM.REGISTER_LIST[OPERAND] : OPERAND
#define CHAR_PTR_CAST(VALUE) ((char*)&VALUE)


#define QUAD_SIZE sizeof(unsigned long)
#define DOUBLE_SIZE sizeof(unsigned int)
#define WORD_SIZE sizeof(unsigned short)
#define BYTE_SIZE sizeof(unsigned char)



typedef struct JRM_DATA
{

        unsigned long REGISTER_LIST[REGISTER_COUNT];

        char* restrict MEMORY_SPACE;

        size_t CODE_INDEX;

        unsigned long OPERAND_1;
        unsigned long OPERAND_2;

        size_t JSM_CURRENT_SOC;

        size_t MEMORY_SPACE_SIZE;
        size_t START_PLUS_HEAP_SIZE;

        size_t TOTAL_SOC;

        unsigned long EXIT_CODE;

        unsigned long NOT_EXIT_REQUESTED;
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

        void MOD_INSTRUCTION();

        void CMPE_INSTRUCTION();

        void CMPH_INSTRUCTION();

        void CMPL_INSTRUCTION();

        void CMPHE_INSTRUCTION();

        void CMPLE_INSTRUCTION();

        void PUSHQ_INSTRUCTION();

        void PUSHD_INSTRUCTION();

        void PUSHW_INSTRUCTION();

        void PUSHB_INSTRUCTION();

        void POPQ_INSTRUCTION();

        void POPD_INSTRUCTION();

        void POPW_INSTRUCTION();

        void POPB_INSTRUCTION();

        void LOADQ_INSTRUCTION();

        void LOADD_INSTRUCTION();

        void LOADW_INSTRUCTION();

        void LOADB_INSTRUCTION();

        void WRITEQ_INSTRUCTION();

        void WRITED_INSTRUCTION();

        void WRITEW_INSTRUCTION();

        void WRITEB_INSTRUCTION();

        void VERFH_INSTRUCTION();

        void JRMCALL_INSTRUCTION();

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
        MOD_INSTRUCTION,
        CMPE_INSTRUCTION,
        CMPH_INSTRUCTION,
        CMPL_INSTRUCTION,
        CMPHE_INSTRUCTION,
        CMPLE_INSTRUCTION,
        PUSHQ_INSTRUCTION,
        PUSHD_INSTRUCTION,
        PUSHW_INSTRUCTION,
        PUSHB_INSTRUCTION,
        POPQ_INSTRUCTION,
        POPD_INSTRUCTION,
        POPW_INSTRUCTION,
        POPB_INSTRUCTION,
        LOADQ_INSTRUCTION,
        LOADD_INSTRUCTION,
        LOADW_INSTRUCTION,
        LOADB_INSTRUCTION,
        WRITEQ_INSTRUCTION,
        WRITED_INSTRUCTION,
        WRITEW_INSTRUCTION,
        WRITEB_INSTRUCTION,
        VERFH_INSTRUCTION,
        JRMCALL_INSTRUCTION

};




void JRM_LOG_ERROR(const char* MESSAGE);

void LOG_PRELOAD_ERROR(const char* MESSAGE);

int JRM__INIT(const char* CODE, size_t BYTECODE_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB);

void JSM__EXIT();


int JRM__INIT(const char* CODE, size_t BYTECODE_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB)
{

        // [SETUP MEMORY SPACE]
        {

                // - ALIGN THE BYTECODE + DATA TO 8 BYTES -
                BYTECODE_SIZE += (BYTECODE_SIZE % QUAD_SIZE != 0) * (QUAD_SIZE - (BYTECODE_SIZE % QUAD_SIZE));


                JRM.MEMORY_SPACE_SIZE = BYTECODE_SIZE + (1024 * 1024 * (STACK_SIZE_MB + HEAP_SIZE_MB));


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


                // - FIND END INSTRUCTION IN CODE -
                for (INDEX = 0; JRM.MEMORY_SPACE[INDEX] != END; INDEX += BYTECODE_STATEMENT_SIZE);


                INDEX += BYTECODE_STATEMENT_SIZE;


                JRM.TOTAL_SOC = INDEX / BYTECODE_STATEMENT_SIZE;


                JRM.REGISTER_LIST[RDP] = INDEX;
                JRM.REGISTER_LIST[RDB] = INDEX;

        }


        // [SET HEAP AND STACK REGISTERS]
        {

                JRM.REGISTER_LIST[RHP] = BYTECODE_SIZE;
                JRM.REGISTER_LIST[RHB] = BYTECODE_SIZE;


                JRM.REGISTER_LIST[RSP] = BYTECODE_SIZE + (1024 * 1024 * HEAP_SIZE_MB);
                JRM.REGISTER_LIST[RSB] = JRM.REGISTER_LIST[RSP];


                JRM.START_PLUS_HEAP_SIZE = JRM.REGISTER_LIST[RSP];

        }


        // [SET / RESET RUNTIME DATA]
        {

                JRM.NOT_EXIT_REQUESTED = TRUE;
                JRM.INCREMENT_STATMENT_INDEX = TRUE;

                JRM.CODE_INDEX = 0;
                JRM.JSM_CURRENT_SOC = 1;

        }


        return JSM_OK;

}


int JRM__RUN(const char* CODE, const size_t CODE_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB)
{

        const int INIT_STATUS = JRM__INIT(CODE, CODE_SIZE, STACK_SIZE_MB, HEAP_SIZE_MB);


        if (INIT_STATUS == JSM_ERROR)
        {

                return JSM_ERROR;

        }


        while (JRM.NOT_EXIT_REQUESTED)
        {

                // [EXECUTE INSTRUCTION]
                {

                        const unsigned long INSTRUCTION = JRM.MEMORY_SPACE[JRM.CODE_INDEX];


                        #if !defined (UNFETTERED)

                                if (INSTRUCTION >= INSTRUCTION_COUNT)
                                {

                                        JRM.EXIT_CODE = 7;


                                        JSM__EXIT();


                                        return JSM_ERROR;
                                }

                        #endif


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
                else if (JRM.EXIT_CODE == 5)
                {

                        printf("PROGRAM EXITED WITH CODE 5 : REGISTER INDEX OUT OF BOUNDS");

                }
                else if (JRM.EXIT_CODE == 7)
                {

                        printf("PROGRAM EXITED WITH CODE 7 : INSTRUCTION INDEX OUT OF BOUNDS");

                }
                else if (JRM.EXIT_CODE == 8)
                {

                        printf("PROGRAM EXITED WITH CODE 8 : OUT OF HEAP MEMORY. CONSIDER RESIZING THE HEAP");

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

        JRM.NOT_EXIT_REQUESTED = FALSE;
        JRM.EXIT_CODE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);

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

        JRM.CODE_INDEX = JRM.REGISTER_LIST[RRS] * BYTECODE_STATEMENT_SIZE;


        JRM.INCREMENT_STATMENT_INDEX = FALSE;

}


void SKIP_INSTRUCTION()
{

        return;

}


void END_INSTRUCTION()
{

        JRM.NOT_EXIT_REQUESTED = FALSE;
        JRM.EXIT_CODE = 3;

}


void JUMP_INSTRUCTION()
{

        const unsigned long STATEMENT_NUMBER = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);;


        #if !defined (UNFETTERED)

                if (STATEMENT_NUMBER == 0 || STATEMENT_NUMBER > JRM.TOTAL_SOC)
                {

                        JRM.EXIT_CODE = 4;
                        JRM.NOT_EXIT_REQUESTED = FALSE;


                        return;

                }

        #endif


        JRM.CODE_INDEX = (STATEMENT_NUMBER - 1) * BYTECODE_STATEMENT_SIZE;


        JRM.INCREMENT_STATMENT_INDEX = FALSE;

}


void PUSHQ_INSTRUCTION()
{

        const unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



        if (CURRENT_RSP + QUAD_SIZE >= JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        *((unsigned long*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = VALUE;



        JRM.REGISTER_LIST[RSP] += QUAD_SIZE;

}


void PUSHD_INSTRUCTION()
{

        const unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



        if (CURRENT_RSP + DOUBLE_SIZE >= JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        *((unsigned int*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = VALUE;



        JRM.REGISTER_LIST[RSP] += DOUBLE_SIZE;

}


void PUSHW_INSTRUCTION()
{

        const unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



        if (CURRENT_RSP + WORD_SIZE >= JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        *((unsigned short*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = VALUE;



        JRM.REGISTER_LIST[RSP] += WORD_SIZE;

}


void PUSHB_INSTRUCTION()
{

        const unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



        if (CURRENT_RSP + BYTE_SIZE >= JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        JRM.MEMORY_SPACE[CURRENT_RSP] = VALUE;


        JRM.REGISTER_LIST[RSP] += BYTE_SIZE;

}


void POPQ_INSTRUCTION()
{

        unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP + 1 < QUAD_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        CURRENT_RSP -= QUAD_SIZE;


        JRM.REGISTER_LIST[REGISTER] = *((unsigned long*)(JRM.MEMORY_SPACE + CURRENT_RSP));


        JRM.REGISTER_LIST[RSP] -= QUAD_SIZE;

}


void POPD_INSTRUCTION()
{

        unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP + 1 < DOUBLE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        CURRENT_RSP -= DOUBLE_SIZE;


        JRM.REGISTER_LIST[REGISTER] = *((unsigned int*)(JRM.MEMORY_SPACE + CURRENT_RSP));


        JRM.REGISTER_LIST[RSP] -= DOUBLE_SIZE;

}


void POPW_INSTRUCTION()
{

        unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP + 1 < WORD_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        CURRENT_RSP -= WORD_SIZE;


        JRM.REGISTER_LIST[REGISTER] = *((unsigned short*)(JRM.MEMORY_SPACE + CURRENT_RSP));


        JRM.REGISTER_LIST[RSP] -= WORD_SIZE;

}


void POPB_INSTRUCTION()
{

        unsigned long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP + 1 < BYTE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        CURRENT_RSP -= BYTE_SIZE;


        JRM.REGISTER_LIST[REGISTER] = JRM.MEMORY_SPACE[CURRENT_RSP];


        JRM.REGISTER_LIST[RSP] -= BYTE_SIZE;

}


void LOADQ_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;
        const unsigned long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (LOAD_INDEX + QUAD_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        JRM.REGISTER_LIST[REGISTER] = *((unsigned long*)(JRM.MEMORY_SPACE + LOAD_INDEX));

}


void LOADD_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;
        const unsigned long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (LOAD_INDEX + DOUBLE_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        JRM.REGISTER_LIST[REGISTER] = *((unsigned int*)(JRM.MEMORY_SPACE + LOAD_INDEX));

}


void LOADW_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;
        const unsigned long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (LOAD_INDEX + WORD_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        JRM.REGISTER_LIST[REGISTER] = *((unsigned short*)(JRM.MEMORY_SPACE + LOAD_INDEX));

}


void LOADB_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;
        const unsigned long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        if (LOAD_INDEX + BYTE_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        JRM.REGISTER_LIST[REGISTER] = JRM.MEMORY_SPACE[LOAD_INDEX];

}


void WRITEQ_INSTRUCTION()
{

        const unsigned long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        if (WRITE_INDEX + QUAD_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        *((unsigned long*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = WRITE_VALUE;

}


void WRITED_INSTRUCTION()
{

        const unsigned long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        if (WRITE_INDEX + DOUBLE_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        *((unsigned int*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = WRITE_VALUE;

}


void WRITEW_INSTRUCTION()
{

        const unsigned long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        if (WRITE_INDEX + WORD_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        *((unsigned short*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = WRITE_VALUE;

}


void WRITEB_INSTRUCTION()
{

        const unsigned long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        if (WRITE_INDEX + BYTE_SIZE > JRM.MEMORY_SPACE_SIZE)
        {

                JRM.EXIT_CODE = 11;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }


        JRM.MEMORY_SPACE[WRITE_INDEX] = WRITE_VALUE;

}


void SET_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        JRM.REGISTER_LIST[REGISTER] = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void ADD_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        JRM.REGISTER_LIST[REGISTER] += CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void SUB_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        JRM.REGISTER_LIST[REGISTER] -= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void MUL_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        JRM.REGISTER_LIST[REGISTER] *= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void DIV_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        JRM.REGISTER_LIST[REGISTER] /= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void MOD_INSTRUCTION()
{

        const unsigned long REGISTER = JRM.OPERAND_1;


        IF_REGISTER_NOT_VALID_ERROR_EXIT(REGISTER);


        JRM.REGISTER_LIST[REGISTER] %= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);

}


void VERFH_INSTRUCTION()
{

        const unsigned long INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
        const unsigned long SIZE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


        if (INDEX + SIZE >= JRM.START_PLUS_HEAP_SIZE)
        {

                JRM.EXIT_CODE = 8;
                JRM.NOT_EXIT_REQUESTED = FALSE;


                return;

        }

}


void JRMCALL_INSTRUCTION()
{

        const unsigned long CURRENT_RJM = JRM.REGISTER_LIST[RJM];


        switch (CURRENT_RJM)
        {

                case (1) : goto PRINT;
                default : return;

        }


        PRINT:
        {

                long VALID = TRUE;


                const unsigned long STARTING_INDEX = JRM.REGISTER_LIST[RJ1];
                unsigned long PRINT_LENGTH = JRM.REGISTER_LIST[RJ2];



                if (STARTING_INDEX >= JRM.MEMORY_SPACE_SIZE)
                {

                        JRM.EXIT_CODE = 11;
                        JRM.NOT_EXIT_REQUESTED = FALSE;

                        return;

                }


                if (PRINT_LENGTH == 0)
                {

                        size_t INDEX;


                        for (INDEX = STARTING_INDEX; JRM.MEMORY_SPACE[INDEX] != '\0' && INDEX < JRM.MEMORY_SPACE_SIZE; INDEX ++, PRINT_LENGTH ++);


                        if (INDEX == JRM.MEMORY_SPACE_SIZE)
                        {

                                VALID = FALSE;

                        }

                }
                else
                {

                        if (STARTING_INDEX + PRINT_LENGTH >= JRM.MEMORY_SPACE_SIZE)
                        {

                                VALID = FALSE;

                        }

                }


                if (!VALID)
                {

                        JRM.EXIT_CODE = 11;
                        JRM.NOT_EXIT_REQUESTED = FALSE;

                        return;

                }


                // [PRINT STRING]
                {

                        const size_t LAST_CHAR_INDEX = STARTING_INDEX + PRINT_LENGTH;

                        const char LAST_CHAR = JRM.MEMORY_SPACE[ LAST_CHAR_INDEX ];


                        JRM.MEMORY_SPACE[ LAST_CHAR_INDEX ] = '\0';

                        printf("%s", JRM.MEMORY_SPACE + STARTING_INDEX);

                        JRM.MEMORY_SPACE[ LAST_CHAR_INDEX ] = LAST_CHAR;

                }


                return;

        }

}


void JRM_LOG_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JRM ERROR]\033[0m : STATEMENT : %lu | %s\n", JRM.JSM_CURRENT_SOC, MESSAGE);

}


void LOG_PRELOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : | %s |\n", MESSAGE);

}
