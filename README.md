# JSM

![C](https://img.shields.io/badge/-00599C?logo=c&logoColor=white&style=plastic)
![JSM](https://img.shields.io/badge/JSM-red?&style=plastic)

<img width="1820" height="640" alt="image" src="https://github.com/user-attachments/assets/dbf969b2-4cc3-4d49-852f-7373ae79ecc7" />


<br>


## The JSM language JRM and assembler


- Written in **C**
- **Register** and **stack** based language
- Memory lightweight JRM
- Direct low level access
- Extremely portable


<br>


---


#### Here's an example of how JSM works, you can compile and run it


```JSM

// THIS PROGRAM PRINTS OUT A - z IN THE ASCII TABLE
// FIRST, LETS SAY X = 65, WHICH IS 'A' IN ASCII

PUSHQ 65;        // PUSH 65 ON THE STACK BY 8 BYTES (X = 65)
POPQ RG1;        // LOAD X INTO REGISTER A
PUSHQ RG1;       // NOW OF COURSE POP IS JUST A SHOWCASE, SO ILL JUST REPUSH IT

SET RG9 RSB;     // SET RG9 TO THE BASE OF THE STACK

                 // [STATEMENT 5]
LOADQ RG1 RG9;   // RELOAD RG1 FROM THE BASE OF THE STACK (RG1 = X)
CMPH RG1 122;    // CHECK IF RG1 (X) IS HIGHER THAN 'z' (END OF A - z)
JUMP 14;         // IF IT IS JUMP TO PRINT IT, OTHERWISE CONTINUE
CMPE RG1 91;     // IF RG1 IS '['
ADD RG1 6;       // ADD IT BY 6 TO SKIP SYMBOLS, ELSE SKIP TO THE NEXT INSTRUCTION (PUSHB RG1;)
PUSHB RG1;       // PUSH THE CHARACTER ONTO THE STACK BY 1 BYTE
ADD RG1 1;       // INCREMENT RG1
WRITEQ RG1 RG9;  // WRITE RG1 BACK TO X
JUMP 5;          // JUMP BACK TO STATEMENT 5


                 // [PRINT IT] (STATEMENT 14)
PUSHB 10;        // PUSH '\n' ONTO THE STACK
PUSHB 0;         // NULL TERMINATE IT
SET RG2 RSB;     // SET RG2 TO THE START OF THE STACK
ADD RG2 8;       // WE NEED TO GET THE START OF THE STRING, SO ADD IT BY THE SIZE OF X (8 BYTES)
SET RJM 1;       // SET JRMCALL MODE TO 1 (PRINT)
SET RJ1 RG2;     // SET ARG1 TO RG2 (START OF STRING)
SET RJ2 0;       // SET THE STRING'S LENGTH TO 0, BECAUSE ITS ALREADY NULL TERMINATED
JRMCALL;         // MOM SAYS WE HAVE SYSCALL AT HOME

EXIT 0;          // EXIT CLEANLY

                 // MAKE SURE TO ALWAYS MARK THE END OF CODE, BECAUSE DATA IS BELOW
END;

LOVE YOU MOM AND DAD!\0

```



#### When compiled, the final bytecode can be visualized like this

<img width="811" height="676" alt="image" src="https://github.com/user-attachments/assets/8e9a3dae-4074-403d-bcb3-22053b019e06" />




---

#### simple hello world program


```JSM

SET RJM 1;
SET RJ1 RDP;
SET RJ2 0;
JRMCALL;
EXIT 0;
END;
HELLO, WORLD!\n\0

```

#### with explaination


```JSM

SET RJM 1;         // SET THE JRMCALL TO PRINT MODE
SET RJ1 RDP;       // SET THE 1ST ARGUMENT TO THE STATIC DATA POINTER
SET RJ2 0;         // THE STRING IS NULL TERMINATED, SO THE LENGTH CAN BE 0
JRMCALL;           // JRMCALL IG
EXIT 0;            // EXIT WITH CODE 0
                   // AFTER END, BELOW IS THE STATIC DATA CONTAINING THE HELLO WORLD STRING
END;
HELLO, WORLD!\n\0

```

#### final output and bytecode visualization 

<img width="812" height="413" alt="image" src="https://github.com/user-attachments/assets/23cdbbcf-9568-4a94-818c-157dd5f4af58" />

