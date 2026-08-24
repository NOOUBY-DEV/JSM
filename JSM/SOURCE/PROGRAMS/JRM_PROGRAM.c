#include "../JRM/JRM.h"
#include <time.h>



#define DEFAULT_HEAP_SIZE_MB 1024
#define DEFAULT_STACK_SIZE_MB 8


long DEBUG = TRUE;


void LOG_JRM_LOAD_ERROR(const char* MESSAGE);

int JRM__READ_JRP_FILE(const char* FILE_PATH, size_t* BYTECODE_SIZE, char** BYTECODE);

void HEXDUMP_BYTECODE(char* BYTECODE, size_t BYTECODE_SIZE);

void HEXDUMP_BYTECODE_WINDOWS(char* BYTECODE, size_t BYTECODE_SIZE);




int main(int ARG_COUNT, char* ARG_STRINGS[])
{

        if (ARG_COUNT < 2)
        {

                LOG_JRM_LOAD_ERROR("MUST HAVE ATLEAST 1 ARGUMENTS");


                return 1;

        }


        char* BYTECODE;
        size_t BYTECODE_SIZE;


        if (JRM__READ_JRP_FILE(ARG_STRINGS[1], &BYTECODE_SIZE, &BYTECODE) == JSM_ERROR)
        {

                return 1;

        }


        if (DEBUG)
        {

                #ifdef _WIN32

                        HEXDUMP_BYTECODE_WINDOWS(BYTECODE, BYTECODE_SIZE);

                #else

                        HEXDUMP_BYTECODE(BYTECODE, BYTECODE_SIZE);

                #endif

        }


        // [RUN]
        {

                struct timespec START, END;


                if (DEBUG)
                {

                        timespec_get(&START, TIME_UTC);

                }


                if (JRM__RUN(BYTECODE, BYTECODE_SIZE, DEFAULT_STACK_SIZE_MB, DEFAULT_HEAP_SIZE_MB) == JSM_ERROR)
                {

                        free(BYTECODE);


                        return 1;

                }


                if (DEBUG)
                {

                        timespec_get(&END, TIME_UTC);


                        double TIME_TAKEN = (END.tv_sec - START.tv_sec) + (END.tv_nsec - START.tv_nsec) / 1000000000.0;


                        printf("[TIME TAKEN] : %.6f SECONDS\n", TIME_TAKEN);

                }

        }


        free(BYTECODE);


        return 0;

}


int JRM__READ_JRP_FILE(const char* FILE_PATH, size_t* BYTECODE_SIZE, char** BYTECODE)
{

        FILE* JSM_FILE = fopen(FILE_PATH, "rb");



        if (JSM_FILE == NULL)
        {

                LOG_JRM_LOAD_ERROR("FAILED TO OPEN .jrp FILE");


                return JSM_ERROR;
        }


        fseek(JSM_FILE, 0, SEEK_END);


        *BYTECODE_SIZE = ftell(JSM_FILE);


        if ((*BYTECODE_SIZE) < 0)
        {

                LOG_JRM_LOAD_ERROR("FAILED TO OPEN .jrp FILE");


                fclose(JSM_FILE);


                return JSM_ERROR;
        }



        rewind(JSM_FILE);


        (*BYTECODE) = (char *)malloc((*BYTECODE_SIZE) + 1);



        if ((*BYTECODE) == NULL)
        {

                LOG_JRM_LOAD_ERROR("FAILED TO OPEN .jrp FILE");


                fclose(JSM_FILE);


                return JSM_ERROR;
        }


        size_t bytes_read = fread((*BYTECODE), 1, (*BYTECODE_SIZE), JSM_FILE);


        if ((*BYTECODE)[*BYTECODE_SIZE - 1] == '\n')
        {

                (*BYTECODE_SIZE) --;

        }



        fclose(JSM_FILE);



        return JSM_OK;

}


void LOG_JRM_LOAD_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JRM ERROR]\033[0m : %s\n", MESSAGE);

}


void HEXDUMP_BYTECODE(char* BYTECODE, size_t BYTECODE_SIZE)
{

        int WAITING_FOR_END = TRUE;


        printf("\n\n");
        printf("IN T1 T2     EMPTY                OPR1                   OPR2\n");
        printf("┌┐ ┌┐ ┌┐ ┌────────────┐ ┌─────────────────────┐ ┌─────────────────────┐\n");



        for (size_t INDEX = 0; INDEX < BYTECODE_SIZE; INDEX++)
        {

                unsigned char CHAR = BYTECODE[INDEX];


                if (CHAR >= 0 && CHAR <= 9)
                {

                        printf("\033[1;31m");

                }
                else
                {

                        printf("\033[0m");

                }



                if (WAITING_FOR_END)
                {

               		const size_t REMAINDER = INDEX % BYTECODE_STATEMENT_SIZE;


	                       	if (REMAINDER == 1 || REMAINDER == 2)
	                        {

	                        	printf("\033[92m");

	                        }


				if (REMAINDER > 2 && REMAINDER < 8)
				{

					printf("\033[37m");

				}

                }



                printf("%02x ", CHAR);


                if ((INDEX + 1) % BYTECODE_STATEMENT_SIZE == 0)
                {


                        if ((BYTECODE[(INDEX + 1) - BYTECODE_STATEMENT_SIZE] == END) && WAITING_FOR_END)
                        {

                                printf("\033[0m\n");
                                printf("───────────────────────────────────────────────────────────────────────\n");
                                printf("//                            DATA BELOW                             //\n");
                                printf("───────────────────────────────────────────────────────────────────────");


                                WAITING_FOR_END = FALSE;

                        }


                        printf("\n");

                }

        }


        printf("\033[0m\n\n\n");

}

void HEXDUMP_BYTECODE_WINDOWS(char* BYTECODE, size_t BYTECODE_SIZE)
{

        int WAITING_FOR_END = TRUE;


        printf("\n\n");
        printf("IN T1 T2     EMPTY                OPR1                   OPR2\n");
        printf("-- -- -- -------------- -------------------- --------------------------\n");
        //////////////// ////////////


        for (size_t INDEX = 0; INDEX < BYTECODE_SIZE; INDEX++)
        {

                unsigned char CHAR = BYTECODE[INDEX];


                if (CHAR >= 0 && CHAR <= 9)
                {

                        printf("\033[1;31m");

                }
                else
                {

                        printf("\033[0m");

                }



                if (WAITING_FOR_END)
                {

               		const size_t REMAINDER = INDEX % BYTECODE_STATEMENT_SIZE;


	                       	if (REMAINDER == 1 || REMAINDER == 2)
	                        {

	                        	printf("\033[92m");

	                        }


				if (REMAINDER > 2 && REMAINDER < 8)
				{

					printf("\033[37m");

				}

                }



                printf("%02x ", CHAR);


                if ((INDEX + 1) % BYTECODE_STATEMENT_SIZE == 0)
                {


                        if ((BYTECODE[(INDEX + 1) - BYTECODE_STATEMENT_SIZE] == END) && WAITING_FOR_END)
                        {

                                printf("\033[0m\n");
                                printf("-----------------------------------------------------------------------\n");
                                printf("//                            DATA BELOW                             //\n");
                                printf("-----------------------------------------------------------------------\n");


                                WAITING_FOR_END = FALSE;

                        }


                        printf("\n");

                }

        }


        printf("\033[0m\n\n\n");

}
