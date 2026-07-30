# JSM

![C](https://img.shields.io/badge/-00599C?logo=c&logoColor=white&style=plastic)
![JSM](https://img.shields.io/badge/JSM-red?&style=plastic)

<img width="1820" height="640" alt="image" src="https://github.com/user-attachments/assets/dbf969b2-4cc3-4d49-852f-7373ae79ecc7" />


<br>


## The JSM language JRM and compiler


- Written in **C**
- **Register** and **stack** based language
- Memory lightweight JRM
- Direct low level access
- Extremely portable


<br>


#### Here's an example of how JSM works, you can compile and run it


```JSM

SET RG1 48;         // SET IS JUST LIKE MOV IN ASSEMBLY
PUSH RG1 8;         // PUSH VALUE OF RG1 ONTO THE STACK BY 8 BYTES
POP RG2 8;          // POP 8 BYTES OFF THE STACK INTO RG2


// -- COMPARE --
CMPE RG1 RG2;       // COMPARE IF RG1 == RG2
JUMP 8;             // JUMP TO STATEMENT 8 IF TRUE 
SKIP;               // OTHERWISE SKIP
ADD RG2 9;          // AFTER SKIP, WE WANT TO ADD RG2 BY 9



// [STATEMENT 8 PART]
{

	SKIP;           // THIS IS STATEMENT 8
					// NOTICE HOW {} AS ORGANIZERS WORK TOO?
	SET RLA RDP;    // SET RLA TO LOAD A VALUE FROM THE STATIC DATA (BOTTOM OF CODE)
	ADD RLA 2;      // WE NOW WANT TO SET IT TO LOAD FROM THERE OFFSET BY 2 BYTES  
	                // SET RLA TO RSP/RSB TO LOAD FROM THE STACK
	LOAD RG3 8;     // LOAD STATIC DATA + 2 INTO RG3 BY 8 BYTES
	
}


EXIT 0;             // REMEMBER TO EXIT PROPERLY, OTHERWISE THE JRM WILL GIVE A WARNING


// ALWAYS END BEFORE THE STATIC DATA, OTHERWISE THE COMPILER WILL THROW AN ERROR!\0
END;


LOVE YOU MOM AND DAD!\0
SKIBIDI TOILET SLICERS\0
ESCAPE CODE TEST \n\123\9\0
UNLESS YOU ADD '\n' INTO THE STATIC DATA,
ANY NEWLINE AND 	TAB CHARACTER IS DISCARED\0

```



#### When compiled, the final bytecode can be visualized like this

<img width="810" height="675" alt="image" src="https://github.com/user-attachments/assets/5dd861eb-afda-457f-80dc-868723d328b3" />



