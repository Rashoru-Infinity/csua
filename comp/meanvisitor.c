
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "visitor.h"
#include "../memory/MEM.h"


#define cs_is_type(type, cs_type) \
  ((type)->basic_type == (cs_type))

#define cs_is_int(type) \
    (cs_is_type(type, CS_INT_TYPE))
#define cs_is_double(type) \
    (cs_is_type(type, CS_DOUBLE_TYPE))
#define cs_is_boolean(type) \
    (cs_is_type(type, CS_BOOLEAN_TYPE))

#define cs_same_type(type1, type2) \
    ((type1)->basic_type == (type2)->basic_type)


char* get_type_name(CS_BasicType type) {
    switch(type) {
        case CS_BOOLEAN_TYPE: {
            return "boolean";            
        }
        case CS_INT_TYPE: {
            return "int";
        }
        case CS_DOUBLE_TYPE: {
            return "double";            
        }
        default: {
            return "untyped";
        }
    }
}

/* 意味解析のログを追加する */
static void add_check_log(const char* str, Visitor* visitor) {
    MeanCheckLogger* log = (MeanCheckLogger*)cs_malloc(sizeof(MeanCheckLogger));
    log->next = NULL;
    log->log_str = (char*)cs_malloc(strlen(str) + 1);
    strcpy(log->log_str, str);
    MeanCheckLogger* p = NULL;
    if (((MeanVisitor*)visitor)->check_log == NULL) {
        ((MeanVisitor*)visitor)->check_log = log;
        return;
    } else {
        for (p = ((MeanVisitor*)visitor)->check_log; p->next; p = p->next);
        p->next = log;
    }
}

void show_mean_error(MeanVisitor* visitor) {
    MeanCheckLogger* p;
    for (p = visitor->check_log; p; p = p->next) {
        fprintf(stderr, "%s\n", p->log_str);
    }
}

static void enter_castexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter castexpr\n");
}
static void leave_castexpr(Expression* expr, Visitor* visitor) { 
    fprintf(stderr, "leave castexpr\n");
}

static void enter_boolexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter boolexpr : %d\n", expr->u.boolean_value);
}
static void leave_boolexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave boolexpr\n");    
    expr->type = cs_create_type_specifier(CS_BOOLEAN_TYPE);
}


static void enter_intexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter intexpr : %d\n", expr->u.int_value);    
}
static void leave_intexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave intexpr\n");
    expr->type = cs_create_type_specifier(CS_INT_TYPE);
}

static void enter_doubleexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter doubleexpr : %f\n", expr->u.double_value);
}
static void leave_doubleexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave doubleexpr\n");            
    expr->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
}

static void enter_identexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter identifierexpr\n");
}

/* 
 * 変数名検索_____ある__ 式内の識別子は変数であると断定
 *            |___ない__ 関数名検索____ある__式内の識別子は関数であると断定
 *                                  |__ない__エラー
 */
static void leave_identexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave identifierexpr %s\n", expr->u.identifier.name);
    Declaration* decl = cs_search_decl_in_block();
    FunctionDeclaration* function = NULL;
    if (!decl) {
        decl = cs_search_decl_global(expr->u.identifier.name);
    }
    
    if (decl) {
        expr->type = decl->type;
        expr->u.identifier.u.declaration = decl;
        expr->u.identifier.is_function = CS_FALSE;
        return;
    }
    function = cs_search_function(expr->u.identifier.name);
    if (function) {
        expr->type = function->type;
        expr->u.identifier.u.function = function;
        expr->u.identifier.is_function = CS_TRUE;
        return;
    }
    
    char message[50];
    sprintf(message, "%d: Cannot find identifier %s", expr->line_number, expr->u.identifier.name);
    add_check_log(message, visitor); 
}

/* 演算子の左右のオペランドの型が未定義になっていないか確認 */
static CS_Boolean check_nulltype_binary_expr(Expression* expr, Visitor* visitor) {
    Expression* left  = expr->u.binary_expression.left;
    Expression* right = expr->u.binary_expression.right;
    if (left->type == NULL) {
        char message[100];
        sprintf(message, "%d: Cannot find left hand type", expr->line_number);
        add_check_log(message, visitor);
    }
    if (right->type == NULL) {
        char message[100];
        sprintf(message, "%d: Cannot find right hand type", expr->line_number);
        add_check_log(message, visitor);
    }
     if ((left->type == NULL) || (right->type == NULL)) {
         return CS_TRUE;
    }
    return CS_FALSE;    
}

/* コードが受理しなかった時のエラーメッセージをロガーに詰める */
static void unacceptable_type_binary_expr(Expression* expr, Visitor* visitor) {
    Expression* left  = expr->u.binary_expression.left;
    Expression* right = expr->u.binary_expression.right;
    char message[100];
    sprintf(message, "%d: type mismatch in arithmetic binary expression left:%s, right:%s", 
            expr->line_number, 
            get_type_name(left->type->basic_type),
            get_type_name(right->type->basic_type));
    
    add_check_log(message, visitor);   
}

/* 
 * 四則演算や余りの演算子の左右のオペランドの型に応じて型変換を行う
 * int <-> int
 * (double)int <-> double
 * double <-> (double)int
 * double <-> double 
 * 式の型を設定する
 */
static void cast_arithmetic_binary_expr(Expression* expr, Visitor* visitor) {
    Expression* left  = expr->u.binary_expression.left;
    Expression* right = expr->u.binary_expression.right;
        
    if (check_nulltype_binary_expr(expr, visitor)) {
        return;
    }        
    
    if (cs_is_int(left->type) && cs_is_int(right->type)) {
        expr->type = cs_create_type_specifier(CS_INT_TYPE);
        return;
    } else if(cs_is_int(left->type) && cs_is_double(right->type)) {
        Expression* cast = cs_create_cast_expression(CS_INT_TO_DOUBLE, expr);
        cast->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
        cast->u.cast_expression.expr = left;
        expr->u.binary_expression.left = cast;
        expr->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
    } else if (cs_is_double(left->type) && cs_is_int(right->type)) {
        Expression* cast = cs_create_cast_expression(CS_INT_TO_DOUBLE, expr);
        cast->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
        cast->u.cast_expression.expr = right;
        expr->u.binary_expression.right = cast;
        expr->type = cs_create_type_specifier(CS_DOUBLE_TYPE);        
    } else if(cs_is_double(left->type) && cs_is_double(right->type)) {
        expr->type = cs_create_type_specifier(CS_DOUBLE_TYPE);        
        return;
    } else {
        unacceptable_type_binary_expr(expr, visitor);       
    }        
    
}

/* arithmetic calculation*/
static void enter_addexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter addexpr : +\n");
}
static void leave_addexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave addexpr\n");
    cast_arithmetic_binary_expr(expr, visitor);
}
static void enter_subexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter subexpr : -\n");
}
static void leave_subexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave subexpr\n");
    cast_arithmetic_binary_expr(expr, visitor);
}
static void enter_mulexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter mulexpr : *\n");
}
static void leave_mulexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave mulexpr\n");
    cast_arithmetic_binary_expr(expr, visitor);    
}
static void enter_divexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter divexpr : /\n");
}
static void leave_divexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave divexpr\n");
    cast_arithmetic_binary_expr(expr, visitor);    
}
static void enter_modexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter modexpr : mod \n");
}
static void leave_modexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave modexpr\n");
    cast_arithmetic_binary_expr(expr, visitor);    
}

/* 
 * 数値型の時は必要に応じてキャスト/数値以外で型が異なる場合や型が未定義の場合はエラー
 * 式の型を設定する
 */
static void compare_type_check(Expression* expr, Visitor* visitor) {
    Expression* left  = expr->u.binary_expression.left;
    Expression* right = expr->u.binary_expression.right;
    
    if (check_nulltype_binary_expr(expr, visitor)) {
        return;
    }
    
    if(cs_same_type(left->type, right->type)) {
        expr->type = cs_create_type_specifier(left->type->basic_type);
        return;
    } else if(cs_is_int(left->type) && cs_is_double(right->type)) {
        // cast left to double        
        Expression* cast = cs_create_cast_expression(CS_INT_TO_DOUBLE, left);
        cast->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
        expr->u.binary_expression.left = cast;
    } else if (cs_is_double(left->type) && cs_is_int(right->type)) {
        // cast right to double
        Expression* cast = cs_create_cast_expression(CS_INT_TO_DOUBLE, right);
        cast->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
        expr->u.binary_expression.right = cast;
    } else {
        unacceptable_type_binary_expr(expr, visitor);
    }
    
    expr->type = cs_create_type_specifier(CS_BOOLEAN_TYPE);
}

static void enter_gtexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter gtexpr : > \n");
}
static void leave_gtexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave gtexpr\n");
    compare_type_check(expr, visitor);
}

static void enter_geexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter geexpr : >= \n");
}
static void leave_geexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave geexpr\n");
    compare_type_check(expr, visitor);
}

static void enter_ltexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter ltexpr : < \n");
}
static void leave_ltexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave ltexpr\n");
    compare_type_check(expr, visitor);
}

static void enter_leexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter leexpr : <= \n");
}
static void leave_leexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave leexpr\n");
    compare_type_check(expr, visitor);
}

/* 
 * 演算子の左右のオペランドの型がbooleanではないときはエラーメッセージをロガーに詰める 
 * 式の型を設定する
 */
static void compare_equality_type_check(Expression* expr, Visitor* visitor) {
    
    if (check_nulltype_binary_expr(expr, visitor)) {
        return;
    }
    Expression* left  = expr->u.binary_expression.left;
    Expression* right = expr->u.binary_expression.right;
    char message[100];    
    if (cs_is_boolean(left->type) && !cs_is_boolean(right->type)) {
        sprintf(message, "%d: type mismatch in equality binary expression left:%s, right:%s", 
                expr->line_number, 
                get_type_name(left->type->basic_type),
                get_type_name(right->type->basic_type));
        
        add_check_log(message, visitor);  
    } else if (!cs_is_boolean(left->type) && cs_is_boolean(right->type)) {
        sprintf(message, "%d: type mismatch in equality binary expression left:%s, right:%s", 
                expr->line_number, 
                get_type_name(left->type->basic_type),
                get_type_name(right->type->basic_type));
        
        add_check_log(message, visitor);  
    } else {
        compare_type_check(expr, visitor);
    }    
}

static void enter_eqexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter eqexpr : == \n");
}
static void leave_eqexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave eqexpr\n");
    compare_equality_type_check(expr, visitor);
}

static void enter_neexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter neexpr : != \n");
}
static void leave_neexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave neexpr\n");
    compare_equality_type_check(expr, visitor);
}

/* 演算子の左右のオペランドがboolean型か確認する */
static void logical_type_check(Expression* expr, Visitor* visitor) {
    if (check_nulltype_binary_expr(expr, visitor)) {
        return;
    }
    
    Expression* left  = expr->u.binary_expression.left;
    Expression* right = expr->u.binary_expression.right;
    
    if (cs_is_boolean(left->type) && cs_is_boolean(right->type)) {
        expr->type = cs_create_type_specifier(CS_BOOLEAN_TYPE);
    } else {
        char message[100]; 
        sprintf(message, "%d: && or || accept only boolean left:%s, right:%s", 
            expr->line_number, 
            get_type_name(left->type->basic_type),
            get_type_name(right->type->basic_type));
    
        add_check_log(message, visitor);   
        
    }
}


static void enter_landexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter landexpr : && \n");
}
static void leave_landexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave landexpr\n");
    logical_type_check(expr, visitor);
}

static void enter_lorexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter lorexpr : || \n");
}
static void leave_lorexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave lorexpr\n");
    logical_type_check(expr, visitor);
}

/*
 * int型以外に++や--を使って以内か確認
 * エラーの時はエラーメッセージをロガーに詰める
 * 式の型を設定する
 */
static void incdec_typecheck(Expression* expr, Visitor* visitor) {
    Expression* idexpr = expr->u.inc_dec;
    char message[100];    
    if (idexpr->type == NULL) {
        sprintf(message, "%d: Cannot find ++ or -- type", expr->line_number);
        add_check_log(message, visitor);        
        return;
    }
   
    if (idexpr->type->basic_type != CS_INT_TYPE) {
        sprintf(message, "%d: Operand is not INT type (%s)", 
                expr->line_number, 
                get_type_name(idexpr->type->basic_type));
        add_check_log(message, visitor);        
        return;
    } else {
        expr->type = idexpr->type;
    }
}
static void enter_incexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter incexpr : ++ \n");
}
static void leave_incexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave incexpr\n");
    incdec_typecheck(expr, visitor);   
}

static void enter_decexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter decexpr : -- \n");
}
static void leave_decexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave decexpr\n");
    incdec_typecheck(expr, visitor);    
}


static void enter_minusexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter minusexpr : - \n");
}

/* 
 * マイナス符号が数値型以外に使われていないか確認
 * エラーならエラーメッセージをロガーに詰める
 * 式の型を設定する
 */
static void leave_minusexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave minusexpr\n");   

    char message[100];
    TypeSpecifier* type = expr->u.minus_expression->type;
    if (type == NULL) {
        sprintf(message, "%d: Cannot find - type", expr->line_number);
        add_check_log(message, visitor);        
        return;
    }
    
    if ((type->basic_type != CS_INT_TYPE) && (type->basic_type != CS_DOUBLE_TYPE)) {
        sprintf(message, "%d: Operand is not INT or DOUBLE type (%s)", 
                expr->line_number, 
                get_type_name(type->basic_type));
        add_check_log(message, visitor);        
        return;        
    } else {
        expr->type = type;    
    }
}

static void enter_lognotexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter lognotexpr : ! \n");
}

/*
 * !(条件の否定)がboolean以外に使われていないか確認
 * エラーならエラーメッセージをロガーに詰める
 * 式の型を設定する
 */
static void leave_lognotexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave lognotexpr\n");

    char message[100];
    TypeSpecifier* type = expr->u.logical_not_expression->type;
    
    if (type == NULL) {
        sprintf(message, "%d: Cannot find ! type", expr->line_number);
        add_check_log(message, visitor);        
        return;
    }
    
    
    if (type->basic_type != CS_BOOLEAN_TYPE) {
        sprintf(message, "%d: Operand is not BOOLEAN type (%s)", 
                expr->line_number, 
                get_type_name(type->basic_type));
        add_check_log(message, visitor);        
        return;    
    } else {
        expr->type = type;    
    }
}

/*
 * 代入式の左右の型を確認する
 * 型がintとdoubleまたはdoubleとintの時はキャストする
 * 型を一致できないときはエラーメッセージをロガーに詰める
 * 式の型を設定する
 */
static Expression* assignment_type_check(TypeSpecifier* ltype, Expression* expr, Visitor* visitor) {
    
    if (ltype == NULL) {
        char message[100];
        sprintf(message, "%d: Cannot find left hand type", expr->line_number);
        add_check_log(message, visitor);
        return expr;
    }
    if (expr->type == NULL) {
        char message[100];
        sprintf(message, "%d: Cannot find right hand type", expr->line_number);
        add_check_log(message, visitor);
        return expr;
    }
    
    
    if (ltype->basic_type == expr->type->basic_type) {
        return expr;
    } else if ((ltype->basic_type == CS_INT_TYPE) && (expr->type->basic_type == CS_DOUBLE_TYPE) ) {
        Expression* cast = cs_create_cast_expression(CS_DOUBLE_TO_INT, expr);
        cast->type = cs_create_type_specifier(CS_INT_TYPE);
        return cast;
    } else if ((ltype->basic_type == CS_DOUBLE_TYPE) && (expr->type->basic_type == CS_INT_TYPE) ) {
        Expression* cast = cs_create_cast_expression(CS_INT_TO_DOUBLE, expr);
        cast->type = cs_create_type_specifier(CS_DOUBLE_TYPE);       
        return cast;
    } else {
        char message[100];
        sprintf(message, "%d: assignment type error %s = %s", 
                expr->line_number,
                get_type_name(ltype->basic_type),
                get_type_name(expr->type->basic_type));
        add_check_log(message, visitor);
    }
    return expr;
}
static void enter_assignexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter assignexpr : %d \n", expr->u.assignment_expression.aope);
}

/* 演算子の左右のオペランドの型を確認して条件に応じて処理を行う */
static void leave_assignexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave assignexpr\n");
    Expression* left  = expr->u.assignment_expression.left;
    Expression* right = expr->u.assignment_expression.right;
    expr->u.assignment_expression.right = assignment_type_check(left->type, right, visitor);
    expr->type = left->type;
    
    
}

static void enter_funccallexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter function call :\n");
}
static void leave_funccallexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave function call\n");
    expr->type = expr->u.function_call_expression.function->type;
}

/* For statement */
static void enter_exprstmt(Statement* stmt, Visitor* visitor) {
    fprintf(stderr, "enter exprstmt :\n");
}
static void leave_exprstmt(Statement* stmt, Visitor* visitor) {
    fprintf(stderr, "leave exprstmt\n");
}

/* 宣言した変数をリストに追加する */
static void enter_declstmt(Statement* stmt, Visitor* visitor) {
    CS_Compiler* compiler = ((MeanVisitor*)visitor)->compiler;
    compiler->decl_list = cs_chain_declaration(compiler->decl_list, stmt->u.declaration_s);
    fprintf(stderr, "enter declstmt\n");    
    
}

/* 必要に応じてキャストnodeを追加する */
static void leave_declstmt(Statement* stmt, Visitor* visitor) {
    fprintf(stderr, "leave declstmt\n");
    Declaration* decl = stmt->u.declaration_s;
    if (decl->initializer != NULL) {
        decl->initializer = assignment_type_check(decl->type, decl->initializer, visitor);
    }
}

/* 意味解析をするvisitorのメモリを確保してcompilerとenter/leave関数を設定して返す */
MeanVisitor* create_mean_visitor() {
    visit_expr* enter_expr_list;
    visit_expr* leave_expr_list;
    visit_stmt* enter_stmt_list;
    visit_stmt* leave_stmt_list;
    
    MeanVisitor* visitor = MEM_malloc(sizeof(MeanVisitor));
    visitor->check_log = NULL;
    visitor->compiler = cs_get_current_compiler();
    if (visitor->compiler == NULL) {
        fprintf(stderr, "Compile is NULL\n");
        exit(1);
    }
    
    
    
    enter_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    leave_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    enter_stmt_list = (visit_stmt*)MEM_malloc(sizeof(visit_stmt) * STATEMENT_TYPE_COUNT_PLUS_ONE);
    leave_stmt_list = (visit_stmt*)MEM_malloc(sizeof(visit_stmt) * STATEMENT_TYPE_COUNT_PLUS_ONE);    
    
    

    enter_expr_list[BOOLEAN_EXPRESSION]       = enter_boolexpr;
    enter_expr_list[INT_EXPRESSION]           = enter_intexpr;
    enter_expr_list[DOUBLE_EXPRESSION]        = enter_doubleexpr;
    enter_expr_list[IDENTIFIER_EXPRESSION]    = enter_identexpr;    
    enter_expr_list[ADD_EXPRESSION]           = enter_addexpr;
    enter_expr_list[SUB_EXPRESSION]           = enter_subexpr;
    enter_expr_list[MUL_EXPRESSION]           = enter_mulexpr;
    enter_expr_list[DIV_EXPRESSION]           = enter_divexpr;
    enter_expr_list[MOD_EXPRESSION]           = enter_modexpr;    
    enter_expr_list[GT_EXPRESSION]            = enter_gtexpr;
    enter_expr_list[GE_EXPRESSION]            = enter_geexpr;
    enter_expr_list[LT_EXPRESSION]            = enter_ltexpr;
    enter_expr_list[LE_EXPRESSION]            = enter_leexpr;
    enter_expr_list[EQ_EXPRESSION]            = enter_eqexpr;
    enter_expr_list[NE_EXPRESSION]            = enter_neexpr;
    enter_expr_list[LOGICAL_AND_EXPRESSION]   = enter_landexpr;
    enter_expr_list[LOGICAL_OR_EXPRESSION]    = enter_lorexpr;
    enter_expr_list[INCREMENT_EXPRESSION]     = enter_incexpr;
    enter_expr_list[DECREMENT_EXPRESSION]     = enter_decexpr;
    enter_expr_list[MINUS_EXPRESSION]         = enter_minusexpr;
    enter_expr_list[LOGICAL_NOT_EXPRESSION]   = enter_lognotexpr;
    enter_expr_list[ASSIGN_EXPRESSION]        = enter_assignexpr;
    enter_expr_list[FUNCTION_CALL_EXPRESSION] = enter_funccallexpr;
    enter_expr_list[CAST_EXPRESSION]          = enter_castexpr;    
    
    enter_stmt_list[EXPRESSION_STATEMENT]     = enter_exprstmt;
    enter_stmt_list[DECLARATION_STATEMENT]    = enter_declstmt;
    
    
    
    leave_expr_list[BOOLEAN_EXPRESSION]       = leave_boolexpr;
    leave_expr_list[INT_EXPRESSION]           = leave_intexpr;
    leave_expr_list[DOUBLE_EXPRESSION]        = leave_doubleexpr;
    leave_expr_list[IDENTIFIER_EXPRESSION]    = leave_identexpr;    
    leave_expr_list[ADD_EXPRESSION]           = leave_addexpr;
    leave_expr_list[SUB_EXPRESSION]           = leave_subexpr;
    leave_expr_list[MUL_EXPRESSION]           = leave_mulexpr;
    leave_expr_list[DIV_EXPRESSION]           = leave_divexpr;
    leave_expr_list[MOD_EXPRESSION]           = leave_modexpr;    
    leave_expr_list[GT_EXPRESSION]            = leave_gtexpr;
    leave_expr_list[GE_EXPRESSION]            = leave_geexpr;
    leave_expr_list[LT_EXPRESSION]            = leave_ltexpr;
    leave_expr_list[LE_EXPRESSION]            = leave_leexpr;
    leave_expr_list[EQ_EXPRESSION]            = leave_eqexpr;
    leave_expr_list[NE_EXPRESSION]            = leave_neexpr;
    leave_expr_list[LOGICAL_AND_EXPRESSION]   = leave_landexpr;
    leave_expr_list[LOGICAL_OR_EXPRESSION]    = leave_lorexpr;
    leave_expr_list[INCREMENT_EXPRESSION]     = leave_incexpr;
    leave_expr_list[DECREMENT_EXPRESSION]     = leave_decexpr;
    leave_expr_list[DECREMENT_EXPRESSION]     = leave_decexpr;
    leave_expr_list[MINUS_EXPRESSION]         = leave_minusexpr;
    leave_expr_list[LOGICAL_NOT_EXPRESSION]   = leave_lognotexpr;
    leave_expr_list[ASSIGN_EXPRESSION]        = leave_assignexpr;
    leave_expr_list[FUNCTION_CALL_EXPRESSION] = leave_funccallexpr;
    leave_expr_list[CAST_EXPRESSION]          = leave_castexpr;
    
    leave_stmt_list[EXPRESSION_STATEMENT]     = leave_exprstmt;
    leave_stmt_list[DECLARATION_STATEMENT]    = leave_declstmt;
    

    ((Visitor*)visitor)->enter_expr_list = enter_expr_list;
    ((Visitor*)visitor)->leave_expr_list = leave_expr_list;
    ((Visitor*)visitor)->enter_stmt_list = enter_stmt_list;
    ((Visitor*)visitor)->leave_stmt_list = leave_stmt_list;
            
    
    

    return visitor;
}
