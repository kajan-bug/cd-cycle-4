#include <stdio.h>
#include <string.h>
#include <ctype.h>

char *ip;                 /* current position in the input */
int error = 0;
int tcount = 0;           /* number of temporaries used */

struct quad {
    char op[3];
    char a1[20], a2[20], res[20];
} q[50];
int nq = 0;

void E(char *out);
void T(char *out);
void F(char *out);

void skip(void) { while (*ip == ' ') ip++; }

void newTemp(char *t) { sprintf(t, "t%d", ++tcount); }

void emit(const char *op, const char *a1, const char *a2, const char *res) {
    strcpy(q[nq].op, op);
    strcpy(q[nq].a1, a1);
    strcpy(q[nq].a2, a2);
    strcpy(q[nq].res, res);
    nq++;
}

void F(char *out) {                         /* F -> ( E ) | id | number */
    skip();
    if (*ip == '(') {
        ip++;
        E(out);
        skip();
        if (*ip == ')') ip++;
        else error = 1;
    }
    else if (isalnum((unsigned char)*ip) || *ip == '_') {
        int i = 0;
        while (isalnum((unsigned char)*ip) || *ip == '_') {
            if (i < 19) out[i++] = *ip;
            ip++;
        }
        out[i] = '\0';
    }
    else {
        error = 1;
        strcpy(out, "?");
    }
}

void T(char *out) {                         /* T -> F { (*|/) F } */
    char left[20], right[20], t[20], op[2] = {0, 0};
    F(left);
    skip();
    while (*ip == '*' || *ip == '/') {
        op[0] = *ip++;
        F(right);
        newTemp(t);
        emit(op, left, right, t);
        strcpy(left, t);
        skip();
    }
    strcpy(out, left);
}

void E(char *out) {                         /* E -> T { (+|-) T } */
    char left[20], right[20], t[20], op[2] = {0, 0};
    T(left);
    skip();
    while (*ip == '+' || *ip == '-') {
        op[0] = *ip++;
        T(right);
        newTemp(t);
        emit(op, left, right, t);
        strcpy(left, t);
        skip();
    }
    strcpy(out, left);
}

int main(void) {
    char s[100], lhs[20], rhs[20];
    int i = 0;

    printf("Enter an assignment (e.g. a=b+c*d): ");
    fgets(s, sizeof s, stdin);
    s[strcspn(s, "\n")] = '\0';

    ip = s;
    skip();
    while (isalnum((unsigned char)*ip) || *ip == '_') {     /* left side name */
        if (i < 19) lhs[i++] = *ip;
        ip++;
    }
    lhs[i] = '\0';
    skip();
    if (i == 0 || *ip != '=') {
        printf("Invalid: expected   id = expression\n");
        return 1;
    }
    ip++;                                                   /* skip '=' */

    E(rhs);
    skip();
    if (error || *ip != '\0') {
        printf("Invalid expression\n");
        return 1;
    }
    emit("=", rhs, "", lhs);

    FILE *fp = fopen("tac.txt", "w");
    printf("\nThree-address code:\n");
    for (int k = 0; k < nq; k++) {
        if (strcmp(q[k].op, "=") == 0) {
            printf("%s = %s\n", q[k].res, q[k].a1);
            if (fp) fprintf(fp, "%s = %s\n", q[k].res, q[k].a1);
        } else {
            printf("%s = %s %s %s\n", q[k].res, q[k].a1, q[k].op, q[k].a2);
            if (fp) fprintf(fp, "%s = %s %s %s\n", q[k].res, q[k].a1, q[k].op, q[k].a2);
        }
    }
    if (fp) fclose(fp);

    printf("\nQuadruples:\n");
    printf("%-6s %-8s %-8s %-8s\n", "Op", "Arg1", "Arg2", "Result");
    for (int k = 0; k < nq; k++)
        printf("%-6s %-8s %-8s %-8s\n", q[k].op, q[k].a1,
               q[k].a2[0] ? q[k].a2 : "-", q[k].res);
    return 0;
}
