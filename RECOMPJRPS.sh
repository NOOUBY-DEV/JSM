#!/bin/bash


mkdir -p OUTPUT


for file in TEST/*.jsm; do

        # REMOVE EXTENSION
        BASE_NAME=$(basename "$file" .jsm)


        ./OUTPUT/JSMC "$file" "JRPS/${BASE_NAME}.jrp"


        # [IF COMPILE STATUS IS 0]
        if [ $? -eq 0 ]; then

                echo "${BASE_NAME} : COMPILE OK"

        else

                echo "${BASE_NAME} : COMPILE FAILED"

        fi

done
