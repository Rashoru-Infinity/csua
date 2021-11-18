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


/* 
 * visitorのメモリexpressionの個数分確保してenter/leave関数のポインタを格納する
 * 現在はINT_EXPRESSIONのみ対応している
 */
Visitor* create_treeview_visitor() {
    visit_expr* enter_expr_list;
    visit_expr* leave_expr_list;
    
    Visitor* visitor = MEM_malloc(sizeof(Visitor));
    enter_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    leave_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);

    enter_expr_list[INT_EXPRESSION] = enter_intexpr;
    leave_expr_list[INT_EXPRESSION] = leave_intexpr;

    visitor->enter_expr_list = enter_expr_list;
    visitor->leave_expr_list = leave_expr_list;
            
    
    

    return visitor;
}

void delete_visitor(Visitor* visitor) {
    MEM_free(visitor->enter_expr_list);
    MEM_free(visitor->leave_expr_list);
    MEM_free(visitor);
}
