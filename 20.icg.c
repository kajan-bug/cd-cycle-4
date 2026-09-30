/*
### Intermediate Code Generation (Three Address Code) for simple expressions
Input   : an assignment such as   a=b+c*d-e/f      or   x=(a+b)*c
Output  : three address code on screen and quadruples (op arg1 arg2 result) in "quad.txt"
          quad.txt is the input file of 21.backend8086.c

### Algorithm (in words):
Grammar used (it gives * and / higher precedence than + and -):
    stmt   -> id = expr
    expr   -> term  { (+|-) term }
    term   -> factor { (*|/) factor }
    factor -> id | number | ( expr )

1. Read the statement and remove spaces.
2. Read the identifier on the left side and the '=' sign.
3. expr():  call term() to get the left operand.
        While the next symbol is + or -:  read the operator, call term() for the right operand,
        create a new temporary (t1, t2, ...), emit  temp = left op right,
        and make the temporary the new left operand.
4. term():  same as expr() but for * and /, and it calls factor() for the operands.
5. factor(): if the symbol is '(' then parse expr() inside and skip ')',
        otherwise read the identifier / number and return its name.
6. After the whole expression is parsed, emit  id = last_result.
7. Every emitted statement is also written to quad.txt as  "op arg1 arg2 result".
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char s[100];
int p = 0, tcount = 0;
FILE *fq;

void error(const char *msg) {
    printf("Error: %s (near position %d)\n", msg, p);
    exit(1);
}

void newtemp(char *t) { sprintf(t, "t%d", ++tcount); }

void emit(const char *op, const char *a1, const char *a2, const char *res) {
    if (strcmp(op, "=") == 0) printf("%s = %s\n", res, a1);
    else printf("%s = %s %s %s\n", res, a1, op, a2);
    fprintf(fq, "%s %s %s %s\n", op, a1, a2, res);
}

void expr(char *r);

void factor(char *r) {
    if (s[p] == '(') {
        p++;
        expr(r);
        if (s[p] == ')') p++;
        else error("missing )");
    } else if (isalnum(s[p])) {
        int k = 0;
        while (isalnum(s[p])) r[k++] = s[p++];
        r[k] = '\0';
    } else error("operand expected");
}

void term(char *r) {
    char l[20], rt[20], t[20];
    factor(l);
    while (s[p] == '*' || s[p] == '/') {
        char op[2] = {s[p++], '\0'};
        factor(rt);
        newtemp(t);
        emit(op, l, rt, t);
        strcpy(l, t);
    }
    strcpy(r, l);
}

void expr(char *r) {
    char l[20], rt[20], t[20];
    term(l);
    while (s[p] == '+' || s[p] == '-') {
        char op[2] = {s[p++], '\0'};
        term(rt);
        newtemp(t);
        emit(op, l, rt, t);
        strcpy(l, t);
    }
    strcpy(r, l);
}

int main() {
    char line[100], lhs[20], res[20];
    int k = 0, m = 0;

    printf("Enter the expression (e.g. a=b+c*d): ");
    fgets(line, sizeof(line), stdin);
    for (k = 0; line[k]; k++)                 // remove spaces / newline
        if (!isspace(line[k])) s[m++] = line[k];
    s[m] = '\0';

    fq = fopen("quad.txt", "w");
    if (!fq) { printf("Cannot create quad.txt\n"); return 1; }

    // left-hand side identifier
    k = 0;
    if (!isalpha(s[p])) error("identifier expected");
    while (isalnum(s[p])) lhs[k++] = s[p++];
    lhs[k] = '\0';
    if (s[p] != '=') error("= expected");
    p++;

    printf("\nThree address code:\n");
    expr(res);
    if (s[p] != '\0') error("unexpected symbol");
    emit("=", res, "-", lhs);

    fclose(fq);
    printf("\nQuadruples written to quad.txt\n");
    return 0;
}

/*
### Sample Input and Output

#### Expression 1
Input:
a=b+c*d-e/f

Output:
Enter the expression (e.g. a=b+c*d): 
Three address code:
t1 = c * d
t2 = b + t1
t3 = e / f
t4 = t2 - t3
a = t4

Quadruples written to quad.txt

quad.txt:
* c d t1
+ b t1 t2
/ e f t3
- t2 t3 t4
= t4 - a

#### Expression 2 (with brackets)
Input:
x=(a+b)*c

Output:
Enter the expression (e.g. a=b+c*d): 
Three address code:
t1 = a + b
t2 = t1 * c
x = t2

Quadruples written to quad.txt

quad.txt:
+ a b t1
* t1 c t2
= t2 - x
*/
