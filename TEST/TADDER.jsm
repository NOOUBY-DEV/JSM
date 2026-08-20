                      // fn main() -> void :

 PUSHQ 0;             // for ( ^^ ulong i = 0 ^^ ... ...)
                
 PUSHQ 10;            // ulong a = 10

 PUSHQ 10;            // ulong b = 10

 PUSHQ 10;            // ulong c = 10

 PUSHQ 0;             // ulong t

 LOADQ RG1 RSB;
  CMPE RG1 100000000; // for (... ^^ i < 100000000 ^^  ...)

  JUMP 26;         

   SET RG7 RSB;       // t = add(a, b, c)
  INCQ RG7;
 LOADQ RG1 RG7;
   SET RA1 RG1;
  INCQ RG7;
 LOADQ RG1 RG7;
   SET RA2 RG1;
  INCQ RG7;
 LOADQ RG1 RG7;
   SET RA3 RG1;
  CALL 30;
   SET RG7 RSB;
   ADD RG7 32;
WRITEQ RRV RG7;

 LOADQ RG1 RSB;       // for (... ... ^^ i++ ^^)
   INC RG1;
WRITEQ RG1 RSB;

  JUMP 5;

   SET RG7 RSB;       // exit t
   ADD RG7 32;
 LOADQ RG1 RG7;
  EXIT RG1;



                      // fn add(a, b, c) -> ulong :

 PUSHQ 0;             // ulong r

   SET RG1 0;         // r = a + b + c
   ADD RG1 RA1;
   ADD RG1 RA2;       
   ADD RG1 RA3;      
   SET RG7 RSB;       
   ADD RG7 24;
WRITEQ RG1 RG7;

   SET RG7 RSB;       // return r
   ADD RG7 24;
 LOADQ RG1 RG7;
   SET RRV RG1;
RETURN;

   END;
