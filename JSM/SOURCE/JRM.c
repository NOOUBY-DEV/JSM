#include "../JSM.h"


// [INCLUDES]
//

        #include <stddef.h>
        #include <stdio.h>
        #include <stdlib.h>


        #if defined(__linux__)
        //

                #include <unistd.h>
                #include <sys/syscall.h>

        //
        #elif defined(__WIN32)
        //

                #include <windows.h>

        //
        #elif defined(__APPLE__)
        //

                #define _DARWIN_C_SOURCE
                #include <unistd.h>
                #include <sys/syscall.h>

        //
        #endif

//




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


#define QUAD_SIZE sizeof(unsigned long long)
#define DOUBLE_SIZE sizeof(unsigned int)
#define WORD_SIZE sizeof(unsigned short)
#define BYTE_SIZE sizeof(unsigned char)



typedef struct JRM_DATA
{

        unsigned long long REGISTER_LIST[REGISTER_COUNT];

        char* restrict MEMORY_SPACE;

        size_t CODE_INDEX;

        unsigned long long OPERAND_1;
        unsigned long long OPERAND_2;

        size_t JSM_CURRENT_SOC;

        size_t MEMORY_SPACE_SIZE;
        size_t START_PLUS_HEAP_SIZE;

        size_t TOTAL_STATEMENT_COUNT;

        unsigned long long EXIT_CODE;

        unsigned long long NOT_EXIT_REQUESTED;
        unsigned long long INCREMENT_STATMENT_INDEX;

}
JRM_DATA;



void JRM_LOG_ERROR(JRM_DATA* JRM, const char* MESSAGE);

void LOG_PRELOAD_ERROR(const char* MESSAGE);

void JSM__EXIT(JRM_DATA* JRM);


int JRM__INIT(JRM_DATA* JRM, const char* CODE, size_t BYTECODE_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB)
{

        // [SETUP MEMORY SPACE]
        {

                // - ALIGN THE BYTECODE + DATA TO 8 BYTES -
                BYTECODE_SIZE += (BYTECODE_SIZE % QUAD_SIZE != 0) * (QUAD_SIZE - (BYTECODE_SIZE % QUAD_SIZE));


                JRM->MEMORY_SPACE_SIZE = BYTECODE_SIZE + (1024 * 1024 * (STACK_SIZE_MB + HEAP_SIZE_MB));


                JRM->MEMORY_SPACE = malloc(JRM->MEMORY_SPACE_SIZE);


                if (JRM->MEMORY_SPACE == NULL)
                {

                        LOG_PRELOAD_ERROR("FAILED TO ALLOCATE PROGRAM MEMORY SPACE");


                        return JSM_ERROR;

                }

        }


        // [COPY CODE INTO MEMORY SPACE]
        {

                for (size_t INDEX = 0; INDEX < BYTECODE_SIZE; INDEX ++)
                {

                        JRM->MEMORY_SPACE[INDEX] = CODE[INDEX];

                }

        }


        // [SETUP REGISTERS]
        {

                for (unsigned long long INDEX = 0; INDEX < REGISTER_COUNT; INDEX ++)
                {

                        JRM->REGISTER_LIST[INDEX] = 0;

                }

        }


        // [SET RDP & RDB]
        {

                size_t INDEX;


                JRM->TOTAL_STATEMENT_COUNT = 0;


                // - FIND END INSTRUCTION IN CODE -
                for (INDEX = 0; JRM->MEMORY_SPACE[INDEX] != END; INDEX += BYTECODE_STATEMENT_SIZE);


                INDEX += BYTECODE_STATEMENT_SIZE;


                JRM->TOTAL_STATEMENT_COUNT = INDEX / BYTECODE_STATEMENT_SIZE;


                JRM->REGISTER_LIST[RDP] = INDEX;
                JRM->REGISTER_LIST[RDB] = INDEX;

        }


        // [SET HEAP AND STACK REGISTERS]
        {

                JRM->REGISTER_LIST[RHP] = BYTECODE_SIZE;
                JRM->REGISTER_LIST[RHB] = BYTECODE_SIZE;


                JRM->REGISTER_LIST[RSP] = BYTECODE_SIZE + (1024 * 1024 * HEAP_SIZE_MB);
                JRM->REGISTER_LIST[RSB] = JRM->REGISTER_LIST[RSP];


                JRM->START_PLUS_HEAP_SIZE = JRM->REGISTER_LIST[RSP];

        }


        // [SET RST]
        {

                #if defined (__linux__)
                {

                        JRM->REGISTER_LIST[RST] = 1;

                }
                #elif defined (__WIN32)
                {

                        JRM->REGISTER_LIST[RST] = 2;

                }
                #elif defined(__APPLE__) && defined(__MACH__)
                {

                        JRM->REGISTER_LIST[RST] = 3;

                }
                #else
                {

                        JRM->REGISTER_LIST[RST] = 0;

                }
                #endif

        }

        // [SET RPA]
        {

                JRM->REGISTER_LIST[RPA] = (unsigned long long)JRM->MEMORY_SPACE;

        }


        // [SET / RESET RUNTIME DATA]
        {

                JRM->NOT_EXIT_REQUESTED = TRUE;
                JRM->INCREMENT_STATMENT_INDEX = TRUE;

                JRM->CODE_INDEX = 0;
                JRM->JSM_CURRENT_SOC = 1;

        }


        return JSM_OK;

}


int JRM__RUN(const char* CODE, const size_t CODE_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB)
{

        JRM_DATA JRM;


        const int INIT_STATUS = JRM__INIT(&JRM, CODE, CODE_SIZE, STACK_SIZE_MB, HEAP_SIZE_MB);


        if (INIT_STATUS == JSM_ERROR)
        {

                return JSM_ERROR;

        }


        while (JRM.NOT_EXIT_REQUESTED)
        {

                const unsigned long long INSTRUCTION = JRM.MEMORY_SPACE[JRM.CODE_INDEX];


                #if !defined (UNFETTERED)

                        if (INSTRUCTION >= INSTRUCTION_COUNT)
                        {

                                JRM.EXIT_CODE = 7;


                                JSM__EXIT(&JRM);


                                return JSM_ERROR;

                        }

                #endif


                // [WRITE BOTH OPERANDS]
                {

                        JRM.OPERAND_1 = *((const unsigned long long*)(JRM.MEMORY_SPACE + JRM.CODE_INDEX + 8));
                        JRM.OPERAND_2 = *((const unsigned long long*)(JRM.MEMORY_SPACE + JRM.CODE_INDEX + 16));

                }


                switch (INSTRUCTION)
                {

                        case (EXIT) :
                        {

                                JRM.NOT_EXIT_REQUESTED = FALSE;
                                JRM.EXIT_CODE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (RETURN) :
                        {

                                const unsigned long long STATEMENT_INDEX = JRM.REGISTER_LIST[RRS];


                                #if !defined (UNFETTERED)

                                        if (STATEMENT_INDEX >= JRM.TOTAL_STATEMENT_COUNT)
                                        {

                                                JRM.EXIT_CODE = 4;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.CODE_INDEX = STATEMENT_INDEX * BYTECODE_STATEMENT_SIZE;
                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                break;

                        }


                        case (END) :
                        {

                                JRM.NOT_EXIT_REQUESTED = FALSE;
                                JRM.EXIT_CODE = 3;


                                break;

                        }


                        case (JUMP) :
                        {

                                const unsigned long long STATEMENT_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                #if !defined (UNFETTERED)

                                        if (STATEMENT_INDEX >= JRM.TOTAL_STATEMENT_COUNT)
                                        {

                                                JRM.EXIT_CODE = 4;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.CODE_INDEX = STATEMENT_INDEX * BYTECODE_STATEMENT_SIZE;


                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                break;

                        }


                        case (CALL) :
                        {

                                const unsigned long long STATEMENT_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                #if !defined (UNFETTERED)

                                        if (STATEMENT_INDEX >= JRM.TOTAL_STATEMENT_COUNT)
                                        {

                                                JRM.EXIT_CODE = 4;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[RRS] = (JRM.CODE_INDEX / BYTECODE_STATEMENT_SIZE) + 1;
                                JRM.CODE_INDEX = STATEMENT_INDEX * BYTECODE_STATEMENT_SIZE;
                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                break;

                        }


                        case (SKIP) :
                        {

                                break;

                        }


                        case (SET) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (ADD) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] += CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (SUB) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] -= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (MUL) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] *= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (DIV) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] /= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (MOD) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] %= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (INC) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] ++;


                                break;

                        }


                        case (DEC) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.REGISTER_LIST[REGISTER] --;


                                break;

                        }


                        case (CMPE) :
                        {

                                const unsigned long long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 != OPERAND_2));


                                break;

                        }


                        case (CMPH) :
                        {

                                const unsigned long long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 <= OPERAND_2));


                                break;

                        }


                        case (CMPL) :
                        {

                                const unsigned long long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 >= OPERAND_2));


                                break;

                        }


                        case (CMPHE) :
                        {

                                const unsigned long long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 < OPERAND_2));


                                break;

                        }


                        case (CMPLE) :
                        {

                                const unsigned long long OPERAND_1 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long OPERAND_2 = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (OPERAND_1 > OPERAND_2));


                                break;

                        }


                        case (PUSHQ) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - QUAD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned long long*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = VALUE;


                                JRM.REGISTER_LIST[RSP] += QUAD_SIZE;


                                break;

                        }


                        case (PUSHD) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - DOUBLE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned int*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = VALUE;


                                JRM.REGISTER_LIST[RSP] += DOUBLE_SIZE;


                                break;

                        }


                        case (PUSHW) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - WORD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned short*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = VALUE;


                                JRM.REGISTER_LIST[RSP] += WORD_SIZE;


                                break;

                        }


                        case (PUSHB) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);



                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - BYTE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.MEMORY_SPACE[CURRENT_RSP] = VALUE;


                                JRM.REGISTER_LIST[RSP] += BYTE_SIZE;


                                break;

                        }


                        case (POPQ) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < QUAD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                CURRENT_RSP -= QUAD_SIZE;


                                JRM.REGISTER_LIST[REGISTER] = *((unsigned long long*)(JRM.MEMORY_SPACE + CURRENT_RSP));


                                JRM.REGISTER_LIST[RSP] -= QUAD_SIZE;


                                break;

                        }


                        case (POPD) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < DOUBLE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                CURRENT_RSP -= DOUBLE_SIZE;


                                JRM.REGISTER_LIST[REGISTER] = *((unsigned int*)(JRM.MEMORY_SPACE + CURRENT_RSP));


                                JRM.REGISTER_LIST[RSP] -= DOUBLE_SIZE;


                                break;

                        }


                        case (POPW) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < WORD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                CURRENT_RSP -= WORD_SIZE;


                                JRM.REGISTER_LIST[REGISTER] = *((unsigned short*)(JRM.MEMORY_SPACE + CURRENT_RSP));


                                JRM.REGISTER_LIST[RSP] -= WORD_SIZE;


                                break;

                        }


                        case (POPB) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long REGISTER = JRM.OPERAND_1;


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < BYTE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                CURRENT_RSP -= BYTE_SIZE;


                                JRM.REGISTER_LIST[REGISTER] = JRM.MEMORY_SPACE[CURRENT_RSP];


                                JRM.REGISTER_LIST[RSP] -= BYTE_SIZE;


                                break;

                        }


                        case (LOADQ) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - QUAD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *((unsigned long long*)(JRM.MEMORY_SPACE + LOAD_INDEX));


                                break;

                        }


                        case (LOADD) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - DOUBLE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *((unsigned int*)(JRM.MEMORY_SPACE + LOAD_INDEX));


                                break;

                        }


                        case (LOADW) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - WORD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *((unsigned short*)(JRM.MEMORY_SPACE + LOAD_INDEX));


                                break;

                        }


                        case (LOADB) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - BYTE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = JRM.MEMORY_SPACE[LOAD_INDEX];


                                break;

                        }


                        case (WRITEQ) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - QUAD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned long long*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = WRITE_VALUE;


                                break;

                        }


                        case (WRITED) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - DOUBLE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned int*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = WRITE_VALUE;


                                break;

                        }


                        case (WRITEW) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - WORD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned short*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = WRITE_VALUE;


                                break;

                        }


                        case (WRITEB) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - BYTE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.MEMORY_SPACE[WRITE_INDEX] = WRITE_VALUE;


                                break;

                        }


                        case (VPLOADQ) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned long long* LOAD_ADDRESS = (const unsigned long long*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPLOADD) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned int* LOAD_ADDRESS = (const unsigned int*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPLOADW) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned short* LOAD_ADDRESS = (const unsigned short*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPLOADB) :
                        {

                                const unsigned long long REGISTER = JRM.OPERAND_1;
                                const unsigned char* LOAD_ADDRESS = (const unsigned char*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                #if !defined (UNFETTERED)

                                        if (REGISTER >= REGISTER_COUNT)
                                        {

                                                JRM.EXIT_CODE = 5;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[REGISTER] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPWRITEQ) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                unsigned long long* WRITE_ADDRESS = (unsigned long long*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = WRITE_VALUE;


                                break;

                        }


                        case (VPWRITED) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                unsigned int* WRITE_ADDRESS = (unsigned int*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (unsigned int)WRITE_VALUE;


                                break;

                        }


                        case (VPWRITEW) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                unsigned short* WRITE_ADDRESS = (unsigned short*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (unsigned short)WRITE_VALUE;


                                break;

                        }


                        case (VPWRITEB) :
                        {

                                const unsigned long long WRITE_VALUE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                unsigned char* WRITE_ADDRESS = (unsigned char*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (unsigned char)WRITE_VALUE;


                                break;

                        }


                        case (VERFH) :
                        {

                                const unsigned long long INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long SIZE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (INDEX >= JRM.START_PLUS_HEAP_SIZE || SIZE >= JRM.START_PLUS_HEAP_SIZE - INDEX)
                                {

                                        JRM.EXIT_CODE = 8;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                break;

                        }


                        case (HALLOC) :
                        {

                                const unsigned long long SIZE = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);
                                const unsigned long long CURRENT_RHP = JRM.REGISTER_LIST[RHP];


                                if (CURRENT_RHP >= JRM.START_PLUS_HEAP_SIZE || SIZE >= JRM.START_PLUS_HEAP_SIZE - CURRENT_RHP)
                                {

                                        JRM.EXIT_CODE = 8;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[RHP] += SIZE;


                                break;

                        }


                        case (JRMCALL) :
                        {

                                const unsigned long long CURRENT_RJM = JRM.REGISTER_LIST[RJM];


                                switch (CURRENT_RJM)
                                {

                                        case (1) : goto PRINT;
                                        default: break;

                                }


                                // [BREAK BEFORE IT REACHES THE GOTOS]
                                {

                                        break;

                                }


                                PRINT:
                                {

                                        long VALID = TRUE;


                                        const unsigned long long STARTING_INDEX = JRM.REGISTER_LIST[RJ1];
                                        unsigned long long PRINT_LENGTH = JRM.REGISTER_LIST[RJ2];



                                        if (STARTING_INDEX >= JRM.MEMORY_SPACE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

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

                                                if (PRINT_LENGTH >= JRM.MEMORY_SPACE_SIZE - STARTING_INDEX)
                                                {

                                                        VALID = FALSE;

                                                }

                                        }


                                        if (!VALID)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }


                                        // [PRINT STRING]
                                        {

                                                const size_t LAST_CHAR_INDEX = STARTING_INDEX + PRINT_LENGTH;

                                                const char LAST_CHAR = JRM.MEMORY_SPACE[ LAST_CHAR_INDEX ];


                                                JRM.MEMORY_SPACE[ LAST_CHAR_INDEX ] = '\0';

                                                printf("%s", JRM.MEMORY_SPACE + STARTING_INDEX);

                                                JRM.MEMORY_SPACE[ LAST_CHAR_INDEX ] = LAST_CHAR;

                                        }


                                        break;

                                }

                        }


                        case (SYSCALL) :
                        {

                                #if !defined (__WIN32)
                                {

                                        JRM.REGISTER_LIST[RSR] = syscall(JRM.REGISTER_LIST[RSM], JRM.REGISTER_LIST[RS1], JRM.REGISTER_LIST[RS2], JRM.REGISTER_LIST[RS3], JRM.REGISTER_LIST[RS4], JRM.REGISTER_LIST[RS5], JRM.REGISTER_LIST[RS6]);

                                }
                                #endif

                        }

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


        JSM__EXIT(&JRM);


        return JSM_OK;

}


void JSM__EXIT(JRM_DATA* JRM)
{

        free(JRM->MEMORY_SPACE);



        // [PRINT EXIT STATUS]
        {

                printf("\033[1;31m[JRM EXIT]\033[0m : ");


                if (JRM->EXIT_CODE == 0)
                {

                        printf("PROGRAM SUCCESSFULLY EXITED WITH CODE 0");

                }
                else if (JRM->EXIT_CODE == 3)
                {

                        printf("PROGRAM EXITED WITH CODE 3 : END STATMENT WAS CALLED, MAKE SURE TO CALL EXIT BEFORE END");

                }
                else if (JRM->EXIT_CODE == 4)
                {

                        printf("PROGRAM EXITED WITH CODE 4 : STATMENT JUMP OUT OF BOUNDS");

                }
                else if (JRM->EXIT_CODE == 5)
                {

                        printf("PROGRAM EXITED WITH CODE 5 : REGISTER INDEX OUT OF BOUNDS");

                }
                else if (JRM->EXIT_CODE == 7)
                {

                        printf("PROGRAM EXITED WITH CODE 7 : INSTRUCTION INDEX OUT OF BOUNDS");

                }
                else if (JRM->EXIT_CODE == 8)
                {

                        printf("PROGRAM EXITED WITH CODE 8 : OUT OF HEAP MEMORY. CONSIDER RESIZING THE HEAP");

                }
                else if (JRM->EXIT_CODE == 11)
                {

                        printf("PROGRAM EXITED WITH CODE 11 : SEGMENTATION FAULT");

                }
                else
                {

                        printf("PROGRAM EXITED WITH CODE %llu", JRM->EXIT_CODE);

                }


                printf("\n");

        }

}


void JRM_LOG_ERROR(JRM_DATA* JRM, const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JRM ERROR]\033[0m : STATEMENT : %lu | %s\n", JRM->JSM_CURRENT_SOC, MESSAGE);

}


void LOG_PRELOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : | %s |\n", MESSAGE);

}
