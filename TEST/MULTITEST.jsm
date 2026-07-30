SET RG1 0;
SET RG2 0;
SET RG3 0;
SET RG4 0;
SET RG8 0;
SET RG9 1000000;


// [S7]
CMPL RG8 RG9;
{
        JUMP 10;
}
// ELSE
{
        JUMP 22;
        SET RG1 RG8;
        SET RG2 RG1;
        ADD RG2 1;
        PUSH RG1 8;
        PUSH RG2 8;
        POP RG3 8;
        POP RG4 8;
        ADD RG3 RG4;
        CMPH RG3 1000000;
        SET RG3 0;
        ADD RG8 1;
        JUMP 7;
}

// [S22]
EXIT 0;


END;
