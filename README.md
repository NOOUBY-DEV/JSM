# JSM

## The JEZSEMBLY language runtime machine and compiler


Its getting late but ill introduce this better tmr, sorry fellows


I wanted to make a stack based ASM like language that can be compiled from a higher level language, and be compiled to bytecode
When it runs, instruction is directly executed in O(1) complexity


for now, the compiler is fully working, so you guys can check that out first, but the rumtime machine is still unsafe, ill keep working on it tmr


<br>


#### But here's an example of how jsm works, you can compile it 


```JSM

SETMODE REG VAL;    // EXPLICITLY SET THE OPERAND TYPES
SET RG1 48;         // SET IS JUST LIKE MOV IN ASSEMBLY
PUSH RG1 8;         // PUSH VALUE OF RG1 ONTO THE STACK BY 8 BYTES
POP RG2 8;          // POP 8 BYTES OFF THE STACK INTO RG2


// -- COMPARE --
SETMODE REG REG;    // WE'RE NOW ABOUT TO COMPARE THE RG1 WITH RG2 REGISTER
CMPE RG1 RG2;       // COMPARE IF RG1 == RG2
JUMP 11;            // JUMP TO STATEMENT 11 IF TRUE 
SKIP;               // OTHERWISE SKIP
SETMODE PREV VAL;   // AFTER SKIP, WE WANT TO ADD A VALUE TO RG2
ADD RG2 9;          // ADD RG2 BY 9



// [STATEMENT 11 PART]
{

	SKIP;           // THIS IS STATEMENT 11
					// NOTICE HOW {} AS ORGANIZERS WORK TOO?
	LOADMODE 1 2;   // LOAD FROM THE STATIC DATA (BOTTOM OF CODE) AT THE 3RD BYTE
	                // LOADMODE 0 4; IF WE WANT TO LOAD FROM THE STACK AT THE 5TH BYTE
	LOAD RG3 8;     // LOAD STATIC DATA + 4 INTO RG3 BY 8 BYTES
	
}


END;		        // ALWAYS END BEFORE THE STATIC DATA, OTHERWISE THE COMPILER WILL THROW AN ERROR!\0


LOVE YOU MOM AND DAD!\0
SKIBIDI TOILET SLICERS\0
ESCAPE CODE TEST \n\123\9\0
UNLESS YOU ADD '\n' INTO THE STATIC DATA,
ANY NEWLINE AND 	TAB CHARACTER IS DISCARED\0

```



#### When compiled, the final bytecode can be visualized like this

<img width="576" height="815" alt="image" src="https://github.com/user-attachments/assets/dea06038-4eb3-4738-a3da-ab44d19ee0a5" />
