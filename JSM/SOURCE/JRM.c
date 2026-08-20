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
                #include <termios.h>
                #include <unistd.h>
                #include <sys/select.h>

        //
        #elif defined(__WIN32)
        //

                #include <windows.h>
                #include <io.h>
                #include <conio.h>
                #include <windows.h>

        //
        #elif defined(__APPLE__)
        //

                #define _DARWIN_C_SOURCE
                #include <unistd.h>
                #include <sys/syscall.h>
                #include <termios.h>
                #include <unistd.h>
                #include <sys/select.h>

        //
        #endif

//


#ifdef _WIN32
    #include <io.h>
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/select.h>
#endif



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


#define DEBUG


#define CAST_OPERAND_TO_TYPE(OPERAND, NUMBER) (*(JRM.BYTECODE + JRM.CODE_INDEX + NUMBER)) ? JRM.REGISTER_LIST[OPERAND] : OPERAND
#define CHAR_PTR_CAST(VALUE) ((char*)&VALUE)


#define QUAD_SIZE sizeof(unsigned long long)
#define DOUBLE_SIZE sizeof(unsigned int)
#define WORD_SIZE sizeof(unsigned short)
#define BYTE_SIZE sizeof(unsigned char)



typedef struct JRM_DATA
{

        unsigned long long REGISTER_LIST[REGISTER_COUNT];

        char* restrict MEMORY_SPACE;
        char* restrict BYTECODE;

        size_t CODE_INDEX;

        unsigned long long OPERAND_1;
        unsigned long long OPERAND_2;

        unsigned long long JSM_CURRENT_SOC;

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

static inline unsigned char PRESSED_KEY();

void INPUT_TO_BUFFER(char* BUFFER, const size_t MAX_LENGTH);


int JRM__INIT(JRM_DATA* JRM, char* restrict CODE, size_t PROGRAM_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB)
{

        size_t BYTECODE_SIZE;


        // [VERIFY AND CLEAN BYTECODE]
        {

                JRM->BYTECODE = CODE;


                if (PROGRAM_SIZE < BYTECODE_STATEMENT_SIZE)
                {

                        LOG_PRELOAD_ERROR("INVALID PROGRAM BINARY SIZE");


                        free(JRM->MEMORY_SPACE);


                        return JSM_ERROR;

                }


                size_t INDEX = 0;


                for (; CODE[INDEX] != END && INDEX < PROGRAM_SIZE; INDEX += BYTECODE_STATEMENT_SIZE)
                {

                        char* BYTECODE_STATEMENT = CODE + INDEX;
                        unsigned long long* OPERANDS = (unsigned long long*)(CODE + INDEX);


                        // [CHECK FOR INVALID CODE]
                        {

                                if (BYTECODE_STATEMENT[0] >= INSTRUCTION_COUNT)
                                {

                                        LOG_PRELOAD_ERROR("INVALID PROGRAM CONTENT");


                                        return JSM_ERROR;

                                }


                                if (BYTECODE_STATEMENT[1] && (OPERANDS[1] >= REGISTER_COUNT))
                                {

                                        LOG_PRELOAD_ERROR("INVALID PROGRAM CONTENT");


                                        return JSM_ERROR;

                                }


                                if (BYTECODE_STATEMENT[2] && (OPERANDS[2] >= REGISTER_COUNT))
                                {

                                        LOG_PRELOAD_ERROR("INVALID PROGRAM CONTENT");


                                        return JSM_ERROR;

                                }

                        }


                        // - CLEAR EMPTY SPACE -
                        for (size_t CLEAR_INDEX = 3; CLEAR_INDEX < 8; CLEAR_INDEX ++)
                        {

                                BYTECODE_STATEMENT[CLEAR_INDEX] = 0;

                        }

                }


                if (INDEX >= PROGRAM_SIZE)
                {

                        LOG_PRELOAD_ERROR("MISSING EOC (END OF CODE) FROM PROGRAM BINARY");


                        return JSM_ERROR;

                }


                BYTECODE_SIZE = INDEX + BYTECODE_STATEMENT_SIZE;

        }


        size_t FULL_DATA_SIZE = (PROGRAM_SIZE - BYTECODE_SIZE);
        FULL_DATA_SIZE += (FULL_DATA_SIZE) % QUAD_SIZE;


        // [SETUP MEMORY SPACE]
        {


                JRM->MEMORY_SPACE_SIZE = FULL_DATA_SIZE + (1024 * 1024 * (STACK_SIZE_MB + HEAP_SIZE_MB));


                JRM->MEMORY_SPACE = malloc(JRM->MEMORY_SPACE_SIZE);


                if (JRM->MEMORY_SPACE == NULL)
                {

                        LOG_PRELOAD_ERROR("FAILED TO ALLOCATE PROGRAM MEMORY SPACE");


                        return JSM_ERROR;

                }

        }


        // [SETUP REGISTERS]
        {

                for (unsigned long long INDEX = 0; INDEX < REGISTER_COUNT; INDEX ++)
                {

                        JRM->REGISTER_LIST[INDEX] = 0;

                }

        }


        // [COPY STATIC DATA]
        {

                for (size_t INDEX = BYTECODE_SIZE; INDEX < PROGRAM_SIZE; INDEX ++)
                {

                        JRM->MEMORY_SPACE[INDEX - BYTECODE_SIZE] = CODE[INDEX];

                }


                JRM->REGISTER_LIST[RDP] = 0;
                JRM->REGISTER_LIST[RDB] = 0;

        }


        // [SET HEAP AND STACK REGISTERS]
        {

                JRM->REGISTER_LIST[RHP] = FULL_DATA_SIZE;
                JRM->REGISTER_LIST[RHB] = FULL_DATA_SIZE;


                JRM->REGISTER_LIST[RSB] = FULL_DATA_SIZE + (1024 * 1024 * HEAP_SIZE_MB);
                JRM->REGISTER_LIST[RSP] = JRM->REGISTER_LIST[RSB];


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
                JRM->JSM_CURRENT_SOC = 0;
                JRM->TOTAL_STATEMENT_COUNT = BYTECODE_SIZE / BYTECODE_STATEMENT_SIZE;

        }


        return JSM_OK;

}


int JRM__RUN(char* restrict CODE, const size_t CODE_SIZE, const size_t STACK_SIZE_MB, const size_t HEAP_SIZE_MB)
{

        JRM_DATA JRM;


        const int INIT_STATUS = JRM__INIT(&JRM, CODE, CODE_SIZE, STACK_SIZE_MB, HEAP_SIZE_MB);


        if (INIT_STATUS == JSM_ERROR)
        {

                return JSM_ERROR;

        }


        while (JRM.NOT_EXIT_REQUESTED)
        {

                const unsigned long long INSTRUCTION = JRM.BYTECODE[JRM.CODE_INDEX];


                // [WRITE BOTH OPERANDS]
                {

                        JRM.OPERAND_1 = *((const unsigned long long*)(JRM.BYTECODE + JRM.CODE_INDEX + 8));
                        JRM.OPERAND_2 = *((const unsigned long long*)(JRM.BYTECODE + JRM.CODE_INDEX + 16));

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

                                const unsigned long long CURRENT_RSB = JRM.REGISTER_LIST[RSB];


                                #if !defined (UNFETTERED)

                                        if (CURRENT_RSB < (QUAD_SIZE * 2) || CURRENT_RSB >= JRM.MEMORY_SPACE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                const unsigned long long STATEMENT_INDEX = *( (unsigned long long*) (JRM.MEMORY_SPACE + CURRENT_RSB - QUAD_SIZE) );


                                #if !defined (UNFETTERED)

                                        if (STATEMENT_INDEX >= JRM.TOTAL_STATEMENT_COUNT)
                                        {

                                                JRM.EXIT_CODE = 4;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                const unsigned long long OLD_RSB = *( (unsigned long long*) (JRM.MEMORY_SPACE + CURRENT_RSB - (QUAD_SIZE * 2)) );


                                #if !defined (UNFETTERED)

                                        if (OLD_RSB >= JRM.MEMORY_SPACE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif



                                JRM.REGISTER_LIST[RSB] = OLD_RSB;
                                JRM.REGISTER_LIST[RSP] = CURRENT_RSB - (QUAD_SIZE * 2);


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
                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];
                                const unsigned long long CURRENT_RSB = JRM.REGISTER_LIST[RSB];



                                #if !defined (UNFETTERED)
                                {

                                        if (STATEMENT_INDEX >= JRM.TOTAL_STATEMENT_COUNT)
                                        {

                                                JRM.EXIT_CODE = 4;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }


                                        const unsigned long long MAX_SIZE = JRM.MEMORY_SPACE_SIZE - (QUAD_SIZE * 2);


                                        if (CURRENT_RSP >= MAX_SIZE || CURRENT_RSB >= MAX_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                }
                                #endif


                                // [AUTO PUSH DATA]
                                {

                                        unsigned long long* QUAD__MEMORY_SPACE__AT_RSP = (unsigned long long*)(JRM.MEMORY_SPACE + CURRENT_RSP);


                                        QUAD__MEMORY_SPACE__AT_RSP[0] = CURRENT_RSB;  // - PUSHQ RSB; -
                                        QUAD__MEMORY_SPACE__AT_RSP[1] = (JRM.CODE_INDEX / BYTECODE_STATEMENT_SIZE) + 1;  // - PUSHQ <RETURN STATEMENT> -


                                        JRM.REGISTER_LIST[RSP] += QUAD_SIZE * 2;  // - AUTO ADD RSP BY 16 -
                                        JRM.REGISTER_LIST[RSB] = JRM.REGISTER_LIST[RSP];  // - AUTO SET RSB TO THE BASE OF THE NEW STACK FRAME -

                                }


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

                                JRM.REGISTER_LIST[JRM.OPERAND_1] = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (ADD) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] += CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (SUB) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] -= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (MUL) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] *= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (DIV) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] /= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (MOD) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] %= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                break;

                        }


                        case (INC) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] ++;


                                break;

                        }


                        case (INCQ) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] += QUAD_SIZE;


                                break;

                        }


                        case (INCD) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] += DOUBLE_SIZE;


                                break;

                        }


                        case (INCW) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] += WORD_SIZE;


                                break;

                        }


                        case (DEC) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] --;


                                break;

                        }


                        case (DECQ) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] -= QUAD_SIZE;


                                break;

                        }


                        case (DECD) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] -= DOUBLE_SIZE;


                                break;

                        }


                        case (DECW) :
                        {

                                JRM.REGISTER_LIST[JRM.OPERAND_1] -= WORD_SIZE;


                                break;

                        }


                        case (CMPE) :
                        {

                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                printf("CMPE | OP1 : %llu | OP2 %llu \n", CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1), CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1) == CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2))
                                {

                                        printf("TRUE \n");

                                }
                                else
                                {

                                        printf("FALSE \n");

                                }


                                unsigned long long INCREMENT = BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1) != CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2)));
                                JRM.CODE_INDEX += INCREMENT;

                                break;

                        }


                        case (CMPH) :
                        {

                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1) <= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2)));


                                break;

                        }


                        case (CMPL) :
                        {

                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1) >= CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2)));


                                break;

                        }


                        case (CMPHE) :
                        {

                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1) < CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2)));


                                break;

                        }


                        case (CMPLE) :
                        {

                                JRM.INCREMENT_STATMENT_INDEX = FALSE;


                                JRM.CODE_INDEX += BYTECODE_STATEMENT_SIZE + (BYTECODE_STATEMENT_SIZE * (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1) > CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2)));


                                break;

                        }


                        case (PUSHQ) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)

                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - QUAD_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                *((unsigned long long*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                JRM.REGISTER_LIST[RSP] += QUAD_SIZE;


                                break;

                        }


                        case (PUSHD) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)

                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - DOUBLE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                *((unsigned int*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                JRM.REGISTER_LIST[RSP] += DOUBLE_SIZE;


                                break;

                        }


                        case (PUSHW) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)

                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - WORD_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                *((unsigned short*)(JRM.MEMORY_SPACE + CURRENT_RSP)) = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                JRM.REGISTER_LIST[RSP] += WORD_SIZE;


                                break;

                        }


                        case (PUSHB) :
                        {

                                const unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)


                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE - BYTE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                JRM.MEMORY_SPACE[CURRENT_RSP] = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                JRM.REGISTER_LIST[RSP] += BYTE_SIZE;


                                break;

                        }


                        case (POPQ) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)

                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < QUAD_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                CURRENT_RSP -= QUAD_SIZE;


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = *((unsigned long long*)(JRM.MEMORY_SPACE + CURRENT_RSP));


                                JRM.REGISTER_LIST[RSP] -= QUAD_SIZE;


                                break;

                        }


                        case (POPD) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)


                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < DOUBLE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                CURRENT_RSP -= DOUBLE_SIZE;


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = *((unsigned int*)(JRM.MEMORY_SPACE + CURRENT_RSP));


                                JRM.REGISTER_LIST[RSP] -= DOUBLE_SIZE;


                                break;

                        }


                        case (POPW) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)


                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < WORD_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                CURRENT_RSP -= WORD_SIZE;


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = *((unsigned short*)(JRM.MEMORY_SPACE + CURRENT_RSP));


                                JRM.REGISTER_LIST[RSP] -= WORD_SIZE;


                                break;

                        }


                        case (POPB) :
                        {

                                unsigned long long CURRENT_RSP = JRM.REGISTER_LIST[RSP];


                                #if !defined (UNFETTERED)

                                        if (CURRENT_RSP >= JRM.MEMORY_SPACE_SIZE || CURRENT_RSP < BYTE_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }

                                #endif


                                CURRENT_RSP -= BYTE_SIZE;


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = JRM.MEMORY_SPACE[CURRENT_RSP];


                                JRM.REGISTER_LIST[RSP] -= BYTE_SIZE;


                                break;

                        }


                        case (LOADQ) :
                        {

                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - QUAD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = *((unsigned long long*)(JRM.MEMORY_SPACE + LOAD_INDEX));


                                break;

                        }


                        case (LOADD) :
                        {

                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - DOUBLE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = *((unsigned int*)(JRM.MEMORY_SPACE + LOAD_INDEX));


                                break;

                        }


                        case (LOADW) :
                        {

                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - WORD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = *((unsigned short*)(JRM.MEMORY_SPACE + LOAD_INDEX));


                                break;

                        }


                        case (LOADB) :
                        {

                                const unsigned long long LOAD_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (LOAD_INDEX > JRM.MEMORY_SPACE_SIZE - BYTE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[JRM.OPERAND_1] = JRM.MEMORY_SPACE[LOAD_INDEX];


                                break;

                        }


                        case (WRITEQ) :
                        {

                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - QUAD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned long long*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (WRITED) :
                        {

                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - DOUBLE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned int*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (WRITEW) :
                        {

                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - WORD_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *((unsigned short*)(JRM.MEMORY_SPACE + WRITE_INDEX)) = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (WRITEB) :
                        {

                                const unsigned long long WRITE_INDEX = CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2);


                                if (WRITE_INDEX > JRM.MEMORY_SPACE_SIZE - BYTE_SIZE)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.MEMORY_SPACE[WRITE_INDEX] = CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (VPLOADQ) :
                        {

                                const unsigned long long* LOAD_ADDRESS = (const unsigned long long*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1)] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPLOADD) :
                        {

                                const unsigned int* LOAD_ADDRESS = (const unsigned int*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1)] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPLOADW) :
                        {

                                const unsigned short* LOAD_ADDRESS = (const unsigned short*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1)] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPLOADB) :
                        {

                                const unsigned char* LOAD_ADDRESS = (const unsigned char*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (LOAD_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                JRM.REGISTER_LIST[CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1)] = *LOAD_ADDRESS;


                                break;

                        }


                        case (VPWRITEQ) :
                        {

                                unsigned long long* WRITE_ADDRESS = (unsigned long long*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1));


                                break;

                        }


                        case (VPWRITED) :
                        {

                                unsigned int* WRITE_ADDRESS = (unsigned int*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (unsigned int)CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (VPWRITEW) :
                        {

                                unsigned short* WRITE_ADDRESS = (unsigned short*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (unsigned short)CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


                                break;

                        }


                        case (VPWRITEB) :
                        {

                                unsigned char* WRITE_ADDRESS = (unsigned char*)(CAST_OPERAND_TO_TYPE(JRM.OPERAND_2, 2));


                                if (WRITE_ADDRESS == NULL)
                                {

                                        JRM.EXIT_CODE = 11;
                                        JRM.NOT_EXIT_REQUESTED = FALSE;


                                        break;

                                }


                                *WRITE_ADDRESS = (unsigned char)CAST_OPERAND_TO_TYPE(JRM.OPERAND_1, 1);


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

                                        case 1 : goto PRINT;
                                        case 2 : goto INPTBUFF;
                                        default : break;

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


                                INPTBUFF:
                                {

                                        const unsigned long long STARTING_INDEX = JRM.REGISTER_LIST[RJ1];
                                        const unsigned long long MAX_INPUT_SIZE = JRM.REGISTER_LIST[RJ2];


                                        if (MAX_INPUT_SIZE == 0)
                                        {

                                                break;

                                        }


                                        if (STARTING_INDEX >= JRM.MEMORY_SPACE_SIZE - MAX_INPUT_SIZE)
                                        {

                                                JRM.EXIT_CODE = 11;
                                                JRM.NOT_EXIT_REQUESTED = FALSE;


                                                break;

                                        }


                                        INPUT_TO_BUFFER(JRM.MEMORY_SPACE + STARTING_INDEX, MAX_INPUT_SIZE);


                                        break;

                                }

                        }


                        case (SYSCALL) :
                        {

                                #if !defined (__WIN32)
                                {

                                        JRM.REGISTER_LIST[RSR] = syscall(JRM.REGISTER_LIST[RSM], JRM.REGISTER_LIST[RS1], JRM.REGISTER_LIST[RS2], JRM.REGISTER_LIST[RS3], JRM.REGISTER_LIST[RS4], JRM.REGISTER_LIST[RS5], JRM.REGISTER_LIST[RS6]);

                                }
                                #elif defined (__MINGW32__)
                                {

                                        // - TODO : IDK BECAUSE WINDOWS DOESNT EVEN HAVE A STANDARD ABI, WHICH PISSES ME OFF -

                                }
                                #endif

                        }

                }


                // [INCREMENT]
                {

                        #if defined (DEBUG)

                                JRM.JSM_CURRENT_SOC =  JRM.CODE_INDEX / BYTECODE_STATEMENT_SIZE;

                        #endif


                        JRM.CODE_INDEX += JRM.INCREMENT_STATMENT_INDEX * BYTECODE_STATEMENT_SIZE;


                        JRM.INCREMENT_STATMENT_INDEX = TRUE;

                }

        }


        JSM__EXIT(&JRM);


        return JSM_OK;

}


void INPUT_TO_BUFFER(char* BUFFER, const size_t MAX_LENGTH)
{

        size_t INDEX = 0;


        BUFFER[0] = '\0';


        fflush(stdout);


        while (TRUE)
        {

                char KEY = PRESSED_KEY();



                if (KEY == '\n')
                {

                        fflush(stdout);


                        break;


                }
                else if (KEY == '\b' || KEY == 127)
                {

                        if (INDEX == 0)
                        {

                                continue;

                        }


                        BUFFER[INDEX] = '\0';


                        printf("\b \b");


                        fflush(stdout);


                        INDEX --;


                }
                else if (INDEX < MAX_LENGTH)
                {

                        BUFFER[INDEX] = KEY;
                        BUFFER[INDEX + 1] = '\0';


                        printf("%c", KEY);


                        fflush(stdout);


                        INDEX ++;

                }

        }


        printf("\n");

}


static inline unsigned char PRESSED_KEY()
{

        struct termios OLD_T, NEW_T;

        char KEY;

        tcgetattr(STDIN_FILENO, &OLD_T);

        NEW_T = OLD_T;

        NEW_T.c_lflag &= ~(ICANON | ECHO);

        tcsetattr(STDIN_FILENO, TCSANOW, &NEW_T);

        KEY = (char)getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &OLD_T);


        return KEY;

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


                #if defined (DEBUG)

                        printf(" | STATEMENT : %llu", JRM->JSM_CURRENT_SOC);

                #endif


                printf("\n");

        }

}


void JRM_LOG_ERROR(JRM_DATA* JRM, const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JRM ERROR]\033[0m : STATEMENT : %llu | %s\n", JRM->JSM_CURRENT_SOC, MESSAGE);

}


void LOG_PRELOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSVM ERROR]\033[0m : | %s |\n", MESSAGE);

}
