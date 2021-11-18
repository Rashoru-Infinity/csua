#include <stdio.h>

#include "visitor.h"
#include "../memory/MEM.h"

static int depth = 0;

static void increment() {
    depth++;
}

static void decrement() {
    depth--;
}

void print_depth() {
    for (int i = 0; i < depth; ++i) {
        fprintf(stderr, "  ");
    }
}

static void enter_boolexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter boolexpr : %d\n", expr->u.boolean_value);
    increment();
}
static void leave_boolexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave boolexpr\n");
}


static void enter_intexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter intexpr : %d\n", expr->u.int_value);
    increment();
}
static void leave_intexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave intexpr\n");
}

static void enter_doubleexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter doubleexpr : %f\n", expr->u.double_value);
    increment();
}
static void leave_doubleexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave doubleexpr\n");            
}

static void enter_identexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter identifierexpr : %s\n", expr->u.identifier.name);
    increment();
}
static void leave_identexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave identifierexpr\n");            
}


static void enter_addexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter addexpr : +\n");
    increment();
}
static void leave_addexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave addexpr\n");
}

static void enter_subexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter subexpr : -\n");
    increment();
}
static void leave_subexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave subexpr\n");
}

static void enter_mulexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter mulexpr : *\n");
    increment();
}
static void leave_mulexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave mulexpr\n");
}

static void enter_divexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter divexpr : /\n");
    increment();
}
static void leave_divexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave divexpr\n");
}

static void enter_modexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter modexpr : mod \n");
    increment();
}
static void leave_modexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave modexpr\n");
}


static void enter_gtexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter gtexpr : > \n");
    increment();
}
static void leave_gtexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave gtexpr\n");
}

static void enter_geexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter geexpr : >= \n");
    increment();
}
static void leave_geexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave geexpr\n");
}

static void enter_ltexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter ltexpr : < \n");
    increment();
}
static void leave_ltexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave ltexpr\n");
}

static void enter_leexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter leexpr : <= \n");
    increment();
}
static void leave_leexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave leexpr\n");
}

static void enter_eqexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter eqexpr : == \n");
    increment();
}
static void leave_eqexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave eqexpr\n");
}

static void enter_neexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter neexpr : != \n");
    increment();
}
static void leave_neexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave neexpr\n");
}

static void enter_landexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter landexpr : && \n");
    increment();
}
static void leave_landexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave landexpr\n");
}

static void enter_lorexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter lorexpr : || \n");
    increment();
}
static void leave_lorexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave lorexpr\n");
}

static void enter_incexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter incexpr : ++ \n");
    increment();   
}
static void leave_incexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave incexpr\n");
}

static void enter_decexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter decexpr : -- \n");
    increment();   
}
static void leave_decexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave decexpr\n");
}

static void enter_minusexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter minusexpr : - \n");
    increment();   
}
static void leave_minusexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave minusexpr\n");
}

static void enter_lognotexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter lognotexpr : ! \n");
    increment();  
}
static void leave_lognotexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave lognotexpr\n");
}

static void enter_assignexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter assignexpr : %d \n", expr->u.assignment_expression.aope);
    increment(); 
}
static void leave_assignexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave assignexpr\n");
}

static void enter_funccallexpr(Expression* expr) {
    print_depth();
    fprintf(stderr, "enter function call :\n");
    increment(); 
}
static void leave_funccallexpr(Expression* expr) {
    decrement();
    print_depth();
    fprintf(stderr, "leave function call\n");
}

/* visitorのメモリexpressionの個数分確保してenter/leave関数のポインタを格納する */
Visitor* create_treeview_visitor() {
    visit_expr* enter_expr_list;
    visit_expr* leave_expr_list;
    
    Visitor* visitor = MEM_malloc(sizeof(Visitor));
    enter_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    leave_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);

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
    

    visitor->enter_expr_list = enter_expr_list;
    visitor->leave_expr_list = leave_expr_list;
            
    
    

    return visitor;
}

void delete_visitor(Visitor* visitor) {
    MEM_free(visitor->enter_expr_list);
    MEM_free(visitor->leave_expr_list);
    MEM_free(visitor);
}
