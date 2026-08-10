#include "../../JSM.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>



void LOG_JSMC_COMPILER_ERROR(const char* MESSAGE);

int EXPORT_BYTECODE_TO_JRP_FILE(const char* BYTECODE, const size_t BYTECODE_SIZE, const char* FILE_PATH);


int main(int ARG_COUNT, char* ARG_STRINGS[])
{

        if (ARG_COUNT < 3)
        {

                LOG_JSMC_COMPILER_ERROR("MUST HAVE ATLEAST 2 ARGUMENTS");


                return 1;

        }


        size_t JSMCODE_SIZE;
        size_t BYTECODE_SIZE;
        char* JSMCODE;
        char* BYTECODE;


        if (JSM__READ_FILE_TO_JSMCODE(ARG_STRINGS[1], &JSMCODE_SIZE, &JSMCODE) == JSM_ERROR)
        {

                return 1;

        }


        if (JSM__CHECK_BYTECODE_SIZE(JSMCODE, JSMCODE_SIZE, &BYTECODE_SIZE) == JSM_ERROR)
        {

                return 1;

        }


        BYTECODE = malloc(BYTECODE_SIZE);


        if (BYTECODE == NULL)
        {

                LOG_JSMC_COMPILER_ERROR("FAILED TO ALLOCATE MEMORY FOR BYTECODE");


                return 1;

        }


        if (JSM__COMPILE_TO_BYTECODE(TRUE, JSMCODE, BYTECODE, JSMCODE_SIZE, &BYTECODE_SIZE) == JSM_ERROR)
        {

                return 1;

        }


        if (EXPORT_BYTECODE_TO_JRP_FILE(BYTECODE, BYTECODE_SIZE, ARG_STRINGS[2]) == JSM_ERROR)
        {

                return 1;

        }


        free(BYTECODE);


        return 0;

}


int JSM__READ_FILE_TO_JSMCODE(const char* FILE_PATH, size_t* JSMCODE_LENGTH, char** JSMCODE)
{

        FILE* JSM_FILE = fopen(FILE_PATH, "rb");



        if (JSM_FILE == NULL)
        {

                LOG_JSMC_COMPILER_ERROR("FAILED TO OPEN .jsm FILE");


                return JSM_ERROR;
        }


        fseek(JSM_FILE, 0, SEEK_END);


        *JSMCODE_LENGTH = ftell(JSM_FILE);


        if ((*JSMCODE_LENGTH) < 0)
        {

                LOG_JSMC_COMPILER_ERROR("FAILED TO OPEN .jsm FILE");


                fclose(JSM_FILE);


                return JSM_ERROR;
        }



        rewind(JSM_FILE);


        (*JSMCODE) = (char *)malloc((*JSMCODE_LENGTH) + 1);



        if ((*JSMCODE) == NULL)
        {

                LOG_JSMC_COMPILER_ERROR("FAILED TO OPEN .jsm FILE");


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


int EXPORT_BYTECODE_TO_JRP_FILE(const char* BYTECODE, const size_t BYTECODE_SIZE, const char* FILE_PATH)
{

        FILE *JRP_FILE = fopen(FILE_PATH, "wb");


        if (JRP_FILE == NULL)
        {

                LOG_JSMC_COMPILER_ERROR("FAILED TO MAKE JRP FILE");


                return JSM_ERROR;

        }


        size_t BYTES_WRITTEN = fwrite(BYTECODE, sizeof(char), BYTECODE_SIZE, JRP_FILE);


        if (BYTES_WRITTEN != BYTECODE_SIZE)
        {

                LOG_JSMC_COMPILER_ERROR("FAILED TO FULLY WRITE INTO JRP FILE");


                return JSM_ERROR;

        }


        fclose(JRP_FILE);


        return JSM_OK;

}


void LOG_JSMC_COMPILER_ERROR(const char* MESSAGE)
{

        fprintf(stderr, "\033[1;31m[JSMC ERROR]\033[0m : %s\n", MESSAGE);

}
