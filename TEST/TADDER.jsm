                      // fn main() -> void :

 PUSHQ 0;             // for ( ^^ ulong i = 0 ^^ ... ...)

 PUSHQ 10;            // ulong a = 10

 PUSHQ 10;            // ulong b = 10

 PUSHQ 10;            // ulong c = 10

 PUSHQ 0;             // ulong t

RLOADQ RG1 0;
  CMPE RG1 100000000; // for (... ^^ i < 100000000 ^^  ...) 00000000

  JUMP 20;


RLOADQ RG1 8;         // t = add(a, b, c)
   SET RA1 RG1;
RLOADQ RG1 16;
   SET RA2 RG1;
RLOADQ RG1 24;
   SET RA3 RG1;
  CALL 22;
RWRITEQ RRV 32;

RLOADQ RG1 0;       // for (... ... ^^ i++ ^^)
   INC RG1;
RWRITEQ RG1 0;


  JUMP 5;


RLOADQ RG1 32;        // exit t
  EXIT RG1;



                      // fn add(a, b, c) -> ulong :

 PUSHQ 0;             // ulong r

   SET RG1 0;         // r = a + b + c
   ADD RG1 RA1;
   ADD RG1 RA2;
   ADD RG1 RA3;
RWRITEQ RG1 0;

RLOADQ RG1 0;        // return r
   SET RRV RG1;
RETURN;

   END;
