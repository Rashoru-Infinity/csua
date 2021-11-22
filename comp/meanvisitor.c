
#include <stdio.h>
#include <stdlib.h>

#include "visitor.h"
#include "../memory/MEM.h"


/* 受け取った式のtreeに対して必要に応じてキャスト式を付けて返す */
static Expression* cast_type(TypeSpecifier* ltype, Expression* expr) {
//    printf("ltype     = %d\n", ltype->basic_type);
//    printf("expr type = %d\n", expr->type->basic_type);
    
    if (ltype->basic_type == expr->type->basic_type) {
        return expr;
    } else if ((ltype->basic_type == CS_INT_TYPE) && (expr->type->basic_type == CS_DOUBLE_TYPE) ) {
        Expression* cast = cs_create_cast_expression(CS_DOUBLE_TO_INT, expr);
        cast->type = cs_create_type_specifier(CS_INT_TYPE);
        return cast;
    } else if ((ltype->basic_type == CS_DOUBLE_TYPE) && (expr->type->basic_type == CS_INT_TYPE) ) {
//       printf("int to double\n");        
        Expression* cast = cs_create_cast_expression(CS_INT_TO_DOUBLE, expr);
        cast->type = cs_create_type_specifier(CS_DOUBLE_TYPE);
        
        return cast;
    } else {
        fprintf(stderr, "Type Error\n");
        exit(1);
    }
    return expr;
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
static void leave_identexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave identifierexpr %s\n", expr->u.identifier.name);
    Declaration* decl = cs_search_decl_in_block();
    if (!decl) {
        decl = cs_search_decl_global(expr->u.identifier.name);
    }
    if (decl) {
        expr->type = decl->type;
    } else {
        fprintf(stderr, "Cannot find lefthand type in meanvisitor\n");
        exit(1);
    }
}

static void enter_addexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter addexpr : +\n");
}
static void leave_addexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave addexpr\n");
}

static void enter_subexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter subexpr : -\n");
}
static void leave_subexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave subexpr\n");
}

static void enter_mulexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter mulexpr : *\n");
}
static void leave_mulexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave mulexpr\n");
}

static void enter_divexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter divexpr : /\n");
}
static void leave_divexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave divexpr\n");
}

static void enter_modexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter modexpr : mod \n");
}
static void leave_modexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave modexpr\n");
}


static void enter_gtexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter gtexpr : > \n");
}
static void leave_gtexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave gtexpr\n");
}

static void enter_geexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter geexpr : >= \n");
}
static void leave_geexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave geexpr\n");
}

static void enter_ltexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter ltexpr : < \n");
}
static void leave_ltexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave ltexpr\n");
}

static void enter_leexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter leexpr : <= \n");
}
static void leave_leexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave leexpr\n");
}

static void enter_eqexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter eqexpr : == \n");
}
static void leave_eqexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave eqexpr\n");
}

static void enter_neexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter neexpr : != \n");
}
static void leave_neexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave neexpr\n");
}

static void enter_landexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter landexpr : && \n");
}
static void leave_landexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave landexpr\n");
}

static void enter_lorexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter lorexpr : || \n");
}
static void leave_lorexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave lorexpr\n");
}

static void enter_incexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter incexpr : ++ \n");
}
static void leave_incexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave incexpr\n");
}

static void enter_decexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter decexpr : -- \n");
}
static void leave_decexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave decexpr\n");
}

static void enter_minusexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter minusexpr : - \n");
}
static void leave_minusexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave minusexpr\n");
}

static void enter_lognotexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter lognotexpr : ! \n");
}
static void leave_lognotexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave lognotexpr\n");
}

static void enter_assignexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter assignexpr : %d \n", expr->u.assignment_expression.aope);
}
static void leave_assignexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave assignexpr\n");
    Expression* left  = expr->u.assignment_expression.left;
    Expression* right = expr->u.assignment_expression.right;
     
    expr->u.assignment_expression.right = cast_type(left->type, right);
    
    
}

static void enter_funccallexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "enter function call :\n");
}
static void leave_funccallexpr(Expression* expr, Visitor* visitor) {
    fprintf(stderr, "leave function call\n");
}

/* For statement */
static void enter_exprstmt(Statement* stmt, Visitor* visitor) {
    fprintf(stderr, "enter exprstmt :\n");
}
static void leave_exprstmt(Statement* stmt, Visitor* visitor) {
    fprintf(stderr, "leave exprstmt\n");
}

static void enter_declstmt(Statement* stmt, Visitor* visitor) {
    CS_Compiler* compiler = ((MeanVisitor*)visitor)->compiler;
    compiler->decl_list = cs_chain_declaration(compiler->decl_list, stmt->u.declaration_s);
    fprintf(stderr, "enter declstmt\n");    
    
}

static void leave_declstmt(Statement* stmt, Visitor* visitor) {
    fprintf(stderr, "leave declstmt\n");
    Declaration* decl = stmt->u.declaration_s;
    if (decl->initializer != NULL) {
        decl->initializer = cast_type(decl->type, decl->initializer);
    }
}


MeanVisitor* create_mean_visitor() {
    visit_expr* enter_expr_list;
    visit_expr* leave_expr_list;
    visit_stmt* enter_stmt_list;
    visit_stmt* leave_stmt_list;
    
    MeanVisitor* visitor = MEM_malloc(sizeof(MeanVisitor));
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
