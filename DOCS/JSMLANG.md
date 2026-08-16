# JSM language

<img width="1820" height="640" alt="Group 6" src="https://github.com/user-attachments/assets/f5b4b400-32e3-4702-a84f-14f4fb74fe36" />



<br>


<br>


---
## Table of contents
* [Introduction](#introduction)
* [Compiling](#compiling)
* [How to program](#how-to-program)
* [Must know tips](#must-know-tips)



<br>


---


## Introduction
the JSM language is an Assembly like language, meant for directly writing JSM bytecode in a human readable format


<img width="355" height="191" alt="image" src="https://github.com/user-attachments/assets/8a5c9366-4e43-4011-a0af-5e2d4f0a37b7" />



<br>


<br>


---
## Compiling
<a name="compiling"></a>

### To compile a .jsm into a .jrp (program binary):



<br>



**1. Ensure you have make (GNU make) installed**
```bash
# - Arch based
sudo pacman -S make

# - Debian based
sudo apt install make

# - Fedora/RHEL based
sudo dnf install make
```


<br>


**2. Clone and get in the repo if you haven't**
```bash
git clone https://github.com/NOOUBY-DEV/JSM.git

cd JSM
```


<br>


**3. Run make**
```bash
make
```


<br>


**4. Compile a .jrp program with JSMC**


> technically the JSMC is a program, but there is also a shortcut in the Makefile


- Program method
```bash
./OUTPUT/JSMC /path/to/jsmfile.jsm path/to/output.jrp
```
- Make shortcut method
```bash
make JSMC /path/to/jsmfile.jsm /path/to/output.jrp
```
- Example
```bash
make JSMC TEST/TEST.jsm COMPILED_JRPS/TEST.jrp
```


<br>


---
## How to program

### The JRM



> The JRM is a **stack** and **register** based VM, so the jsm language allows you to access both of them directly


<br>


### 1. Syntax

<br>

**There are only 3 types of statements in JSM**

```JSM
// FULL STATEMENT
INST OPR1 OPR2;

// HALF STATEMENT
INST OPR1;

// SINGLE INSTRUCTION
INST;
```

<br>


**Conditionals**

```JSM
CMPE RG1 10;  // EVALUATES IF RG1 IS 10
ADD RG1 4;    // THIS EXECUTES IF TRUE (WILL EXECUTE NEXT INSTRUCTION)
SUB RG1 4;    // THIS EXECUTED IF FALSE

// REGISTERS WORK TOO
CMPE RG1 RG2; // EVALUATES IF RG1 IS RG2
ADD RG1 4;    // THIS EXECUTES IF TRUE (WILL EXECUTE NEXT INSTRUCTION)
SUB RG1 4;    // THIS EXECUTED IF FALSE
```

<br>


### 2. Instructions


<br>


**Standalone intructions**


| Instruction | What it does | Example 1 | Example 2 |
|--- |--- |--- |--- |
| **SET** | Sets a register to a value | ```SET RG1 14;``` | ```SET RG1 RG2;``` | 
| **ADD** | Adds a register by a value | ```ADD RG1 14;``` | ```ADD RG1 RG2;``` |
| **SUB** | Subtracts a register by a value | ```SUB RG1 14;``` | ```SUB RG1 RG2;``` |
| **MUL** | Multiplies a register by a value | ```MUL RG1 14;``` | ```MUL RG1 RG2;``` |
| **DIV** | Divides a register by a value | ```DIV RG1 14;``` | ```DIV RG1 RG2;``` |
| **MOD** | Modulos a register by a value to its remainder | ```MOD RG1 14;``` | ```MOD RG1 RG2;``` |
| **JUMP** | Jumps to a statement (0-based) | ```JUMP 14;``` | ```JUMP RG1;``` |
| **SKIP** | Skips to the next instruction | ```SKIP;``` | \ |
| **CALL**| Jumps to a statement and sets register **RRS** to the next statment | ```CALL 14;``` | ```CALL RG1;``` |
| **RETURN** | Returns/Jumps to the statement from register **RRS** | ```RETURN;``` | \ |
| **EXIT** | **```MANDATORY!```** Exits the program with a code | ```EXIT 0;``` | ```EXIT RG1;``` |
| **END**| **```MANDATORY!```** Marks the **end of code**, data section is below | ```END;``` | \ |
| **JRMCALL** | Calls the **JRM** for OS level operations (eg. print) | ```JRMCALL;``` | \ |
| **VERFH** | Bounds check if **index + size** of the heap to collide the stack | ```VERFH RHP 1024;``` | ```VERFH RG1 RG2;``` |


<br>

**Conditional intructions**


> **Conditional instructions work with both numbers and registers**


| Instruction | Conventional symbol |
|--- |--- |
| **CMPE** | ```==``` |
| **CMPH** | ```>``` |
| **CMPL** | ```<``` |
| **CMPHE** | ```>=``` |
| **CMPLE** | ```<=``` |


<br>

**Memory instructions**

> **PUSH, POP, LOAD, WRITE series**


- **PUSH**

> Pushes a value onto the stack relative to **RSP**, **RSP** is automatically incremented by the no. of **Bytes**

| Instruction | Bytes | Example 1 | Example 2 |
|--- | :---: |--- |--- |
| **PUSHQ** | 8 | `PUSHQ 14;` | `PUSHQ RG1;` |
| **PUSHD** | 4 | `PUSHD 14;` | `PUSHD RG1;` |
| **PUSHW** | 2 | `PUSHW 14;` | `PUSHW RG1;` |
| **PUSHB** | 1 | `PUSHB 14;` | `PUSHB RG1;` |


---


- **POP**

> Pops a value off the stack to a register relative to **RSP**, **RSP** is automatically subtracted by the no. of **Bytes**

| Instruction | Bytes | Example |
|--- | :---: |--- |
| **POPQ** | 8 | `POPQ RG1;` |
| **POPD** | 4 | `POPD RG1;` |
| **POPW** | 2 | `POPW RG1;` |
| **POPB** | 1 | `POPB RG1;` |


---

- **WRITE**

> Writes in the program memory space by a number of **Bytes** into a register (operand 1) starting from the index (operand 2)

| Instruction | Bytes | Example 1 | Example 2 |
|--- | :---: |--- |--- |
| **WRITEQ** | 8 | `WRITEQ 14 RHP;` | `WRITEQ RG1 RSB;` |
| **WRITED** | 4 | `WRITED 14 RHP;` | `WRITED RG1 RSB;` |
| **WRITEW** | 2 | `WRITEW 14 RHP;` | `WRITEW RG1 RSB;` |
| **WRITEB** | 1 | `WRITEB 14 RHP;` | `WRITEB RG1 RSB;` |


---


- **LOAD**

> Loads a number of **Bytes** into a register (operand 1) starting from the index (operand 2) in the program memory space

| Instruction | Bytes | Example |
|--- | :---: |--- |
| **LOADQ** | 8 | `LOADQ RG1 RDP;` |
| **LOADD** | 4 | `LOADD RG1 RDP;` |
| **LOADW** | 2 | `LOADW RG1 RDP;` |
| **LOADB** | 1 | `LOADB RG1 RDP;` |


<br>


### 3. Registers

> **All registers can be freely modified, but with great power comes great responsibility**


| Register | Use case | Detailed |
| :--- | :--- | :--- |
| **RSP** | **`STACK`** | The stack pointer |
| **RSB** | **`STACK`** | The stack base pointer |
| **RHP** | **`HEAP`** | The heap pointer |
| **RHB** | **`HEAP`** | The heap base pointer |
| **RDP** | **`STATIC DATA`** | The static data pointer |
| **RDB** | **`STATIC DATA`** | The static data base pointer |
| **RLA** | **`GENERAL`, `PLACEHOLDER`** | `LOAD` address/index |
| **RWA** | **`GENERAL`, `PLACEHOLDER`** | `WRITE` address/index |
| **RRS** | **`RETURN`** | The jump statement number for `RETURN`|
| **RRV** | **`GENERAL`, `PLACEHOLDER`** | `RETURN`'s value |
| **RG1** | **`GENERAL`** | \ |
| **RG2** | **`GENERAL`** | \ |
| **RG3** | **`GENERAL`** | \ |
| **RG4** | **`GENERAL`** | \ |
| **RG5** | **`GENERAL`** | \ |
| **RG6** | **`GENERAL`** | \ |
| **RG7** | **`GENERAL`** | \ |
| **RG8** | **`GENERAL`** | \ |
| **RG9** | **`GENERAL`** | \ |
| **RJM** | **`JRMCALL`**| **`JRMCALL`** mode |
| **RJ1** | **`JRMCALL`**| **`JRMCALL`** argument 1 |
| **RJ2** | **`JRMCALL`**| **`JRMCALL`** argument  2|
| **RJ3** | **`JRMCALL`**| **`JRMCALL`** argument 3 |


<br>


---
## Must know tips


<br>


- **Every JSM program should have THIS statement at the end of code**


> If you dont add `END;` JSMC will give you a compiler error


```JSM
SET RG1 14;
EXIT 0;

END; <---- THIS
```


<br>


- **Make sure the program exits cleanly**


Wrong

```JSM
SET RG1 14;

END;
```


Correct

```JSM
SET RG1 14;
EXIT 0;
END;
```

> If the program hits `END` but never hits `EXIT`, the JRM will exit with code 3
