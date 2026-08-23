PUSHB 10;
PUSHQ 10;
PUSHD 10;
PUSHD 10;



// - LOAD ALL FROM MEMORY -
{
        RLOADQ RG1 1;

        SET RG8 RSB;
        ADD RG8 9;
        LOADD RG2 RG8;

        POPD RG3;

        ADD RG1 RG2;
        ADD RG1 RG3;
}
// - -------------------- -


EXIT RG1;


END;
