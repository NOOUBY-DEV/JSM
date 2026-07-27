#include "../JSM/JSM.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>





char* JSMCODE;
char* BYTECODE;


size_t JSMCODE_LENGTH, BYTECODE_LENGTH;






int main()
{

        // [GET .jsm FILE AND COMPILE]
        {

                int OPEN_STATUS = JSM__READ_FILE_TO_JSMCODE("/home/noouby/Documents/JSM_TESTFILES/TEST1.jsm", &JSMCODE_LENGTH, &JSMCODE);


                if (OPEN_STATUS == 0)
                {

                        return 0;

                }


                int SIZE_CHECK_STATUS = JSM__CHECK_BYTECODE_SIZE(JSMCODE, JSMCODE_LENGTH, &BYTECODE_LENGTH);


                if (SIZE_CHECK_STATUS == JSM_ERROR)
                {

                        return 0;

                }


                BYTECODE = malloc(BYTECODE_LENGTH);


                int COMPILE_STATUS = JSM__COMPILE_TO_BYTECODE(JSMCODE, BYTECODE, JSMCODE_LENGTH, &BYTECODE_LENGTH);



                if (COMPILE_STATUS == 0)
                {

                        return 0;

                }

        }


        // [PRINT HEX BYTES]
        {

                int TO_PRINT_DATA_LINE = FALSE;


                printf("\n\n┌┐ ┌─────────────────────┐ ┌─────────────────────┐\n");


                for (size_t INDEX = 0; INDEX < BYTECODE_LENGTH; INDEX ++)
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


                        printf("%02x ", CHAR);


                        if ((INDEX + 1) % 17 == 0)
                        {

                                if (TO_PRINT_DATA_LINE)
                                {

                                        printf("\033[0m\n──────────────────────────────────────────────────");


                                        TO_PRINT_DATA_LINE = FALSE;

                                }

                                if (BYTECODE[INDEX + 1] == END)
                                {

                                        TO_PRINT_DATA_LINE = TRUE;

                                }

                                printf("\n");

                        }

                }

                printf("\033[0m\n\n\n");

        }



        //JSM__RUN(BYTECODE, 2);



        free(JSMCODE);
        free(BYTECODE);


        return 0;

}
