#include <stdio.h>
#include <ctype.h>
#include <string.h>
char op_stack[20];
char val_stack[20][10];
int op_top = -1, val_top = -1, temp_count = 1;
void push_op(char c) { op_stack[++op_top] = c; }
char pop_op() { return op_stack[op_top--]; }
void push_val(char *val) { strcpy(val_stack[++val_top], val); }
void pop_val(char *val) { strcpy(val, val_stack[val_top--]); }
int precedence(char op) {
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0; 
}
void generate_3ac() {
    char op[2], arg1[10], arg2[10], temp[10];
    op[0] = pop_op();
    op[1] = '\0';
    
    pop_val(arg2);
    pop_val(arg1);
    
    sprintf(temp, "t%d", temp_count++);
    printf("%s = %s %s %s\n", temp, arg1, op, arg2);
    push_val(temp);
}
int main() {
    char expr[50] = "a+b*c";
    char result_var[10] = "x";
    
    printf("Input Expression: %s = %s\n\n", result_var, expr);
    printf("Generated Intermediate Code (Three-Address Code):\n");
    printf("--------------------------------------------------\n");
    for (int i = 0; expr[i] != '\0'; i++) {
        if (isalnum(expr[i])) {
            char val[2] = {expr[i], '\0'};
            push_val(val);
        } else {
            while (op_top != -1 && precedence(op_stack[op_top]) >= precedence(expr[i])) {
                generate_3ac();
            }
            push_op(expr[i]);
        }
    }
    while (op_top != -1) {
        generate_3ac();
    }
    char final_temp[10];
    pop_val(final_temp);
    printf("%s = %s\n", result_var, final_temp);
    return 0;
}