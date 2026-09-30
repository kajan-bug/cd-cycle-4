/*
### Back end of the compiler : Three Address Code -> 8086 Assembly
Input   : quad.txt produced by 20.icg.c   (each line:  op arg1 arg2 result)
Output  : "output.asm" - a complete program that can be assembled and run
          with an 8086 assembler (emu8086 / MASM / TASM)

### Algorithm (in words):
1. Open quad.txt and read every quadruple (op, arg1, arg2, result) into an array.
2. Build the symbol table (variables of the data segment):
   - Go through the quadruples in order.
   - An operand that is not a number and not yet in the table is an INPUT variable:
     ask the user for its value and store it.
   - A result name not yet in the table is added with initial value 0.
3. Write the assembly header:  .MODEL SMALL, .STACK, .DATA (one DW per variable), .CODE,
   and the two instructions that load the data segment into DS.
4. For every quadruple write the code below (AX is the working register):
      +   :  MOV AX,arg1 ; ADD AX,arg2 ; MOV result,AX
      -   :  MOV AX,arg1 ; SUB AX,arg2 ; MOV result,AX
      *   :  MOV AX,arg1 ; MOV BX,arg2 ; MUL BX ; MOV result,AX
      /   :  MOV AX,arg1 ; MOV DX,0 ; MOV BX,arg2 ; DIV BX ; MOV result,AX
      =   :  MOV AX,arg1 ; MOV result,AX
5. After the last quadruple, load the final result in AX, call the PRINT procedure
   (prints AX in decimal) and terminate with  MOV AH,4CH / INT 21H.
6. Write the PRINT procedure and  END START.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX 50

char op[MAX][3], a1[MAX][10], a2[MAX][10], res[MAX][10];
char sym[MAX * 3][10];
int  val[MAX * 3], nsym = 0, n = 0;

int lookup(const char *name) {
    for (int i = 0; i < nsym; i++)
        if (strcmp(sym[i], name) == 0) return i;
    return -1;
}

void addsym(const char *name, int v) {
    strcpy(sym[nsym], name);
    val[nsym++] = v;
}

// operand that must be a variable (not a number, not the '-' placeholder)
void checkOperand(const char *name) {
    if (strcmp(name, "-") == 0 || isdigit(name[0])) return;
    if (lookup(name) == -1) {
        int v;
        printf("Enter the value of %s: ", name);
        scanf("%d", &v);
        addsym(name, v);
    }
}

int main() {
    FILE *fin = fopen("quad.txt", "r");
    FILE *fout = fopen("output.asm", "w");
    if (!fin) { printf("Cannot open quad.txt (run 20.icg first)\n"); return 1; }
    if (!fout) { printf("Cannot create output.asm\n"); return 1; }

    // Step 1: read all quadruples
    while (n < MAX && fscanf(fin, "%s %s %s %s", op[n], a1[n], a2[n], res[n]) == 4) n++;
    fclose(fin);

    // Step 2: symbol table
    for (int i = 0; i < n; i++) {
        checkOperand(a1[i]);
        checkOperand(a2[i]);
        if (lookup(res[i]) == -1) addsym(res[i], 0);
    }

    // Step 3: header and data segment
    fprintf(fout, ".MODEL SMALL\n.STACK 100H\n.DATA\n");
    for (int i = 0; i < nsym; i++)
        fprintf(fout, "    %s DW %d\n", sym[i], val[i]);
    fprintf(fout, ".CODE\nSTART:\n    MOV AX, @DATA\n    MOV DS, AX\n\n");

    // Step 4: code for every quadruple
    for (int i = 0; i < n; i++) {
        if (strcmp(op[i], "=") == 0) fprintf(fout, "    ; %s = %s\n", res[i], a1[i]);
        else fprintf(fout, "    ; %s = %s %s %s\n", res[i], a1[i], op[i], a2[i]);
        if (strcmp(op[i], "+") == 0) {
            fprintf(fout, "    MOV AX, %s\n    ADD AX, %s\n    MOV %s, AX\n", a1[i], a2[i], res[i]);
        } else if (strcmp(op[i], "-") == 0) {
            fprintf(fout, "    MOV AX, %s\n    SUB AX, %s\n    MOV %s, AX\n", a1[i], a2[i], res[i]);
        } else if (strcmp(op[i], "*") == 0) {
            fprintf(fout, "    MOV AX, %s\n    MOV BX, %s\n    MUL BX\n    MOV %s, AX\n", a1[i], a2[i], res[i]);
        } else if (strcmp(op[i], "/") == 0) {
            fprintf(fout, "    MOV AX, %s\n    MOV DX, 0\n    MOV BX, %s\n    DIV BX\n    MOV %s, AX\n", a1[i], a2[i], res[i]);
        } else if (strcmp(op[i], "=") == 0) {
            fprintf(fout, "    MOV AX, %s\n    MOV %s, AX\n", a1[i], res[i]);
        }
        fprintf(fout, "\n");
    }

    // Step 5: print final result and exit
    if (n > 0) fprintf(fout, "    MOV AX, %s\n    CALL PRINT\n", res[n - 1]);
    fprintf(fout, "    MOV AH, 4CH\n    INT 21H\n\n");

    // Step 6: PRINT procedure (prints AX as a decimal number)
    fprintf(fout,
        "PRINT PROC\n"
        "    MOV CX, 0\n"
        "    MOV BX, 10\n"
        "NEXT:\n"
        "    MOV DX, 0\n"
        "    DIV BX\n"
        "    PUSH DX\n"
        "    INC CX\n"
        "    CMP AX, 0\n"
        "    JNE NEXT\n"
        "SHOW:\n"
        "    POP DX\n"
        "    ADD DL, 30H\n"
        "    MOV AH, 02H\n"
        "    INT 21H\n"
        "    LOOP SHOW\n"
        "    RET\n"
        "PRINT ENDP\n"
        "END START\n");

    fclose(fout);
    printf("8086 code written to output.asm\n");
    return 0;
}

/*
### Sample Input and Output

#### a=b+c*d (quad.txt from 20.icg.c; keyboard values c=2, d=3, b=4; the program prints 10)
Input:
quad.txt:
* c d t1
+ b t1 t2
= t2 - a

Keyboard input: 2 3 4

Output:
Enter the value of c: Enter the value of d: Enter the value of b: 8086 code written to output.asm

output.asm:
.MODEL SMALL
.STACK 100H
.DATA
    c DW 2
    d DW 3
    t1 DW 0
    b DW 4
    t2 DW 0
    a DW 0
.CODE
START:
    MOV AX, @DATA
    MOV DS, AX

    ; t1 = c * d
    MOV AX, c
    MOV BX, d
    MUL BX
    MOV t1, AX

    ; t2 = b + t1
    MOV AX, b
    ADD AX, t1
    MOV t2, AX

    ; a = t2
    MOV AX, t2
    MOV a, AX

    MOV AX, a
    CALL PRINT
    MOV AH, 4CH
    INT 21H

PRINT PROC
    MOV CX, 0
    MOV BX, 10
NEXT:
    MOV DX, 0
    DIV BX
    PUSH DX
    INC CX
    CMP AX, 0
    JNE NEXT
SHOW:
    POP DX
    ADD DL, 30H
    MOV AH, 02H
    INT 21H
    LOOP SHOW
    RET
PRINT ENDP
END START
*/
