#include "../../JSM.h"


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


int JSM__READ_FILE_TO_JSMCODE(const char* FILE_PATH, size_t* JSMCODE_LENGTH, char** JSMCODE);


int JSM__CHECK_BYTECODE_SIZE(char* JSMCODE, const size_t JSMCODE_LENGTH, size_t* BYTECODE_SIZE);


int JSM__COMPILE_TO_BYTECODE(const long IS_COMPILE_MODE, char* JSMCODE, char* BYTECODE, const size_t JSMCODE_SIZE, size_t* BYTECODE_SIZE);
