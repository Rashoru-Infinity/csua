
#include <stdio.h>
#include <stdlib.h>

#include "visitor.h"

static void traverse_expr_children(Expression* expr, Visitor *visitor);

/* 式をtraverseする */
void traverse_expr(Expression* expr, Visitor* visitor) {
    printf("traverse_expr\n");
    if (expr) {
		/* EXPRESSIONに対応する関数のポインタが与えられていない場合は何もできないのでプログラムを終了する */
        if (visitor->enter_expr_list[expr->kind] == NULL) {
            fprintf(stderr, "enter->type(%d) it null\n", expr->kind);
            exit(1);
        }
		/* EXPRESSIONに対応するenterの関数を実行する */
        visitor->enter_expr_list[expr->kind](expr);
        traverse_expr_children(expr, visitor);
		/* EXPRESSIONに対応するleaveの関数を実行する */
        visitor->leave_expr_list[expr->kind](expr);

    }    
}

static void traverse_expr_children(Expression* expr, Visitor *visitor) {
    switch(expr->kind) {
        case INT_EXPRESSION: {
            break;
        }
        default:
            fprintf(stderr, "No such expr->kind %d\n", expr->kind);
    }
}
