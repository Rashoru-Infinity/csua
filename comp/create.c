#include <stdio.h>
#include "csua.h"
#include "../memory/MEM.h"

static MEM_Storage storage;

static void init_storage() {
    if (storage == NULL) { 
        storage = MEM_open_storage(0);
        printf("init_storage\n");
    }
}

/* nodeのメモリを確保してexpressionの種類を設定する */
static Expression* cs_create_expression(ExpressionKind ekind) {
    init_storage();
    Expression *expr = (Expression*)MEM_storage_malloc(storage, sizeof(Expression));    
    expr->kind = ekind;
    return expr;
}
/* intのtreeのnodeを生成する */
Expression* cs_create_int_expression(int v) {
    Expression* expr = cs_create_expression(INT_EXPRESSION);
    expr->u.int_value = v;
    return expr;
}

void delete_storage() {
    if (storage != NULL) {
        MEM_dispose(storage);
    }
}

/* doubleのtreeのnodeを生成する */
Expression* cs_create_double_expression(double v) {
    Expression* expr = cs_create_expression(DOUBLE_EXPRESSION);
    expr->u.double_value = v;
    return expr;
}

/* booleanのtreeのnodeを生成する */
Expression* cs_create_boolean_expression(CS_Boolean v) {
    Expression* expr = cs_create_expression(BOOLEAN_EXPRESSION);
    expr->u.boolean_value = v;
    return expr;
}

/* 変数名等のtreeのnodeを生成する */
Expression* cs_create_identifier_expression(char* identifier) {
    Expression* expr = cs_create_expression(IDENTIFIER_EXPRESSION);
    expr->u.identifier.name = identifier;
    return expr;
}

/* ++, --のtreeのnodeを生成する */
Expression* cs_create_inc_dec_expression(Expression* id_expr, ExpressionKind inc_dec) {
    Expression* expr = cs_create_expression(inc_dec);
    expr->u.inc_dec = id_expr;
    return expr;
}

// args is argument not yet
/* 関数呼び出し用のtreeのnodeを生成する 関数の引数をvoidポインタ型で渡す */
Expression* cs_create_function_call_expression(Expression* function, void* args) {
    Expression* expr = cs_create_expression(FUNCTION_CALL_EXPRESSION);
    expr->u.function_call_expression.function = function;
    return expr;
}

/* マイナスのtreeのnodeを生成する */
Expression* cs_create_minus_expression(Expression* operand) {
    Expression* expr = cs_create_expression(MINUS_EXPRESSION);
    expr->u.minus_expression = operand;
    return expr;
}

/* 条件の否定のtreeのnodeを生成する */
Expression* cs_create_logical_not_expression(Expression* operand) {
    Expression* expr = cs_create_expression(LOGICAL_NOT_EXPRESSION);
    expr->u.logical_not_expression = operand;
    return expr;
}

Expression* cs_create_binary_expression(ExpressionKind kind, Expression* left, Expression* right) {
    Expression* expr = cs_create_expression(kind);
    expr->u.binary_expression.left = left;
    expr->u.binary_expression.right = right;
    return expr;   
}

/* 代入のtreeのnodeを生成する */
Expression* cs_create_assignment_expression(Expression *left, AssignmentOperator aope, Expression* operand) {
    Expression* expr = cs_create_expression(ASSIGN_EXPRESSION);
    expr->u.assignment_expression.aope = aope;
    expr->u.assignment_expression.left = left;
    expr->u.assignment_expression.right = operand;
    return expr;            
}
        
        
        
        

