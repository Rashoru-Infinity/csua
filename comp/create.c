#include <stdio.h>
#include <string.h>
#include "csua.h"
#include "../memory/MEM.h"

static MEM_Storage storage = NULL;

static void init_storage() {    
    if (storage == NULL) { 
#ifdef STORAGE                
        storage = MEM_open_storage(0);
        printf("init_storage\n");
#else
        storage = cs_get_current_compiler()->storage;
        
#endif
    }
}

/* size分メモリを確保する */
static void* cs_malloc(size_t size) {
    init_storage();
    return MEM_storage_malloc(storage, size);
}

/* nodeのメモリを確保してexpressionの種類を設定する */
static Expression* cs_create_expression(ExpressionKind ekind) {
    Expression* expr = (Expression*)cs_malloc(sizeof(Expression));    
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
#ifdef STORAGE            
    if (storage != NULL) {
        MEM_dispose(storage);
    }
#endif            
}

/* expressionのchainの末尾に新たなexpressionを追加する */
ExpressionList* cs_chain_expression_list(ExpressionList* list, Expression* expr) {
    ExpressionList* p = list;
    ExpressionList* nlist= (ExpressionList*)MEM_storage_malloc(storage, sizeof(ExpressionList));    
    nlist->next = NULL;
    nlist->expression = expr;    
    if (p != NULL) {
        while (p->next) p = p->next;
        p->next = nlist;
        return list;
    } 
    return nlist;
    
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

/* +-*\/%のtreeのnodeを生成する */
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
        
/* 変数名を確保したしたメモリにコピーする */ 
char* cs_create_identifier(const char* str) {
    char* new_char;
    new_char = (char*)cs_malloc(strlen(str) + 1);
    strcpy(new_char, str);
    return new_char;
}

/* For Statement */
/* 文のtreeのnodeを生成する */
static Statement* cs_create_statement(StatementType type) {
    //Expression* expr = (Expression*)cs_malloc(sizeof(Expression));   
    Statement* stmt = (Statement*)cs_malloc(sizeof(Statement));
    stmt->type = type;
    return stmt;    
}

/* 文中の式を設定する */
Statement* cs_create_expression_statement(Expression* expr) {
    Statement* stmt = cs_create_statement(EXPRESSION_STATEMENT);
    stmt->u.expression_s = expr;
    return stmt;
}


/* データ型のtreeのnodeを生成する */
static TypeSpecifier* cs_create_type_specifier(CS_BasicType type) {
    TypeSpecifier* ts = (TypeSpecifier*)cs_malloc(sizeof(TypeSpecifier));
    ts->basic_type = type;

    return ts;
}

/* 宣言文でのデータ型/変数名/初期値(expression)を生成したtreeのnodeに設定する */
static Declaration* cs_create_declaration(CS_BasicType type, char* name, Expression* initializer) {
    Declaration* decl = (Declaration*)cs_malloc(sizeof(Declaration));
    decl->type = cs_create_type_specifier(type);
    decl->name = name;
    decl->initializer = initializer;
    decl->index = -1;
    return decl;        
}

/* 宣言文のtreeのnodeを生成と変数の型/名前/初期値(expression)を設定 */
Statement* cs_create_declaration_statement(CS_BasicType type, char* name, Expression* initializer) {
    Statement* stmt = cs_create_statement(DECLARATION_STATEMENT);
    stmt->u.declaration_s = cs_create_declaration(type, name, initializer);   
    return stmt;
}

/* 文のリストの要素を生成して設定する */
StatementList* cs_create_statement_list(Statement* stmt) {
    StatementList* stmt_list = (StatementList*)cs_malloc(sizeof(StatementList));
    stmt_list->stmt = stmt;
    stmt_list->next = NULL;
    return stmt_list;
}

/* リストの末尾に新たな文を追加する */
StatementList* cs_chain_statement_list(StatementList* stmt_list, Statement* stmt) {
    StatementList* p = NULL;
    StatementList* nstmt_list = cs_create_statement_list(stmt);
    if (stmt_list == NULL) {
//        fprintf(stderr, "stmt_list is null\n");
        return nstmt_list;
    }   
//    fprintf(stderr, "stmt_list is NOT null\n");
    for (p = stmt_list; p->next; p = p->next);
    p->next = nstmt_list;
    
    return stmt_list;
}
