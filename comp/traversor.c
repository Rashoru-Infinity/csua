
#include <stdio.h>
#include <stdlib.h>

#include "visitor.h"

static void traverse_expr_children(Expression* expr, Visitor* visitor);
static void traverse_stmt_children(Statement*  stmt, Visitor* visitor);


/* 式をtraverseする */
void traverse_expr(Expression* expr, Visitor* visitor) {
    if (expr) {
		/* EXPRESSIONに対応する関数のポインタが与えられていない場合は何もできないのでプログラムを終了する */
        if (visitor->enter_expr_list[expr->kind] == NULL) {
            fprintf(stderr, "enter->type(%d) is null\n", expr->kind);
            exit(1);
        }
		/* EXPRESSIONに対応するenterの関数を実行する */
        visitor->enter_expr_list[expr->kind](expr);
        traverse_expr_children(expr, visitor);
		/* EXPRESSIONに対応するleaveの関数を実行する */
        visitor->leave_expr_list[expr->kind](expr);

    }    
}

void traverse_stmt(Statement* stmt, Visitor* visitor) {
    if (stmt) {
        if (visitor->enter_stmt_list[stmt->type] == NULL) {
            fprintf(stderr, "enter->type(%d) is null\n", stmt->type);
            exit(1);
        }
        visitor->enter_stmt_list[stmt->type](stmt);
        traverse_stmt_children(stmt, visitor);
        visitor->leave_stmt_list[stmt->type](stmt);        
    }
}

static void traverse_stmt_children(Statement* stmt, Visitor* visitor) {
    switch(stmt->type) {
        case EXPRESSION_STATEMENT: {
            traverse_expr(stmt->u.expression_s, visitor);
          break;  
        }
        case DECLARATION_STATEMENT: {
            traverse_expr(stmt->u.declaration_s->initializer, visitor);
            break;
        }
        default: {
            fprintf(stderr, "No such stmt->type %d\n", stmt->type);
        }
    }
}

/* プリミティブ型や変数のexpression以外はtraverse_exprを呼び出してastを末端まで探索する */
static void traverse_expr_children(Expression* expr, Visitor *visitor) {
    switch(expr->kind) {
        case BOOLEAN_EXPRESSION:
        case IDENTIFIER_EXPRESSION:
        case DOUBLE_EXPRESSION:
        case INT_EXPRESSION: {
            break;
        }

        case INCREMENT_EXPRESSION:
        case DECREMENT_EXPRESSION: {
            traverse_expr(expr->u.inc_dec, visitor);
            break;
        }
        case MINUS_EXPRESSION: {
            traverse_expr(expr->u.minus_expression, visitor);
            break;
        }
        case LOGICAL_NOT_EXPRESSION: {
            traverse_expr(expr->u.logical_not_expression, visitor);
            break;
        }
        case ASSIGN_EXPRESSION: {
            traverse_expr(expr->u.assignment_expression.left, visitor);
            traverse_expr(expr->u.assignment_expression.right, visitor);
            break;
        }
        case FUNCTION_CALL_EXPRESSION: {
            traverse_expr(expr->u.function_call_expression.function, visitor);
            break;
        }
        case LOGICAL_AND_EXPRESSION:
        case LOGICAL_OR_EXPRESSION:            
        case LT_EXPRESSION:
        case LE_EXPRESSION:
        case GT_EXPRESSION:
        case GE_EXPRESSION:
        case EQ_EXPRESSION:
        case NE_EXPRESSION:            
        case MOD_EXPRESSION:
        case DIV_EXPRESSION:
        case MUL_EXPRESSION:
        case SUB_EXPRESSION:
        case ADD_EXPRESSION: {
            if (expr->u.binary_expression.left) {
                traverse_expr(expr->u.binary_expression.left, visitor);
            }
            if (expr->u.binary_expression.right) {
                traverse_expr(expr->u.binary_expression.right, visitor);
            }
            break;
        }
        default:
            fprintf(stderr, "No such expr->kind %d\n", expr->kind);
    }
}
