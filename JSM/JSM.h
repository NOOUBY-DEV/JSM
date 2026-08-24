#define JSM
#ifdef JSM


#define BYTECODE_STATEMENT_SIZE 24


#define TRUE 1
#define FALSE 0
#define NONE 0
#define JSM_OK 1
#define JSM_ERROR 0



enum INSTRUCTIONS_ENUM
{
        EXIT,
        RETURN,
        END,
        JUMP,
        CALL,
        SKIP,
        SET,
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        INC,
        INCQ,
        INCD,
        INCW,
        DEC,
        DECQ,
        DECD,
        DECW,
        CMPE,
        CMPH,
        CMPL,
        CMPHE,
        CMPLE,
        PUSHQ,
        PUSHD,
        PUSHW,
        PUSHB,
        POPQ,
        POPD,
        POPW,
        POPB,
        LOADQ,
        LOADD,
        LOADW,
        LOADB,
        WRITEQ,
        WRITED,
        WRITEW,
        WRITEB,
        RLOADQ,
        RLOADD,
        RLOADW,
        RLOADB,
        RWRITEQ,
        RWRITED,
        RWRITEW,
        RWRITEB,
        VPLOADQ,
        VPLOADD,
        VPLOADW,
        VPLOADB,
        VPWRITEQ,
        VPWRITED,
        VPWRITEW,
        VPWRITEB,
        VERFH,
        HALLOC,
        JRMCALL,
        SYSCALL,
        INSTRUCTION_COUNT
};


enum REGISTERS_ENUM
{

        RSP,
        RSB,
        RHP,
        RHB,
        RDP,
        RDB,
        RLA,
        RWA,
        RRS,
        RRV,
        RG1,
        RG2,
        RG3,
        RG4,
        RG5,
        RG6,
        RG7,
        RG8,
        RG9,
        RA1,
        RA2,
        RA3,
        RA4,
        RA5,
        RA6,
        RA7,
        RA8,
        RA9,
        RJM,
        RJR,
        RJ1,
        RJ2,
        RJ3,
        RJ4,
        RJ5,
        RJ6,
        RSM,
        RSR,
        RS1,
        RS2,
        RS3,
        RS4,
        RS5,
        RS6,
        RST,
        RPA,
        REGISTER_COUNT

};


#define REG 1
#define VAL 0

#define OPERAND_1_TYPE_INDEX 0
#define OPERAND_2_TYPE_INDEX 1


#endif
