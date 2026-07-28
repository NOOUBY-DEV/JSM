#include "../../JSM/JSM.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>





char* JSMCODE;
char* BYTECODE;


size_t JSMCODE_LENGTH, BYTECODE_SIZE;






int main()
{

        // [GET .jsm FILE AND COMPILE]
        {

                int OPEN_STATUS = JSM__READ_FILE_TO_JSMCODE("/home/noouby/JSM/TESTS/TEST1/TEST1.jsm", &JSMCODE_LENGTH, &JSMCODE);


                if (OPEN_STATUS == 0)
                {

                        return 0;

                }


                int SIZE_CHECK_STATUS = JSM__CHECK_BYTECODE_SIZE(JSMCODE, JSMCODE_LENGTH, &BYTECODE_SIZE);


                if (SIZE_CHECK_STATUS == JSM_ERROR)
                {

                        return 0;

                }


                BYTECODE = malloc(BYTECODE_SIZE);


                int COMPILE_STATUS = JSM__COMPILE_TO_BYTECODE(JSMCODE, BYTECODE, JSMCODE_LENGTH, &BYTECODE_SIZE);



                if (COMPILE_STATUS == 0)
                {

                        return 0;

                }

        }


        // [PRINT HEX BYTES]
        {

                int WAITING_FOR_END = TRUE;


                printf("\n\n");
                printf("IN           OPR1                   OPR2\n");
                printf("┌┐ ┌─────────────────────┐ ┌─────────────────────┐\n");



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


                        printf("%02x ", CHAR);


                        if ((INDEX + 1) % 17 == 0)
                        {


                                if ((BYTECODE[(INDEX + 1) - 17] == END) && WAITING_FOR_END)
                                {

                                        printf("\033[0m\n");
                                        printf("──────────────────────────────────────────────────\n");
                                        printf("//                  DATA BELOW                  //\n");
                                        printf("──────────────────────────────────────────────────");


                                        WAITING_FOR_END = FALSE;

                                }


                                printf("\n");

                        }

                }


                printf("\033[0m\n\n\n");

        }



        JRM__RUN(BYTECODE, BYTECODE_SIZE, 2);



        free(JSMCODE);
        free(BYTECODE);


        return 0;

}
