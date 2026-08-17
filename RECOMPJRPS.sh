#!/bin/bash


cd "$(dirname "$0")" || exit 1


mkdir -p OUTPUT


for file in TEST/*.jsm; do

        # REMOVE EXTENSION
        BASE_NAME=$(basename "$file" .jsm)

        make LAZYCOMP ${BASE_NAME}

        # [IF COMPILE STATUS IS 0]
        if [ $? -eq 0 ]; then

                echo "${BASE_NAME} : COMPILE OK"

        else

                echo "${BASE_NAME} : COMPILE FAILED"

        fi

done
