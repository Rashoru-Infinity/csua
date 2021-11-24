#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "csua.h"
#include "visitor.h"
#include "../svm/svm.h"

/* オペランドのサイズを取得する */
static size_t get_opsize(OpcodeInfo *op) {
    size_t size = strlen(op->parameter);
    size *= 2;    
    return size;
}

/* 命令と引数からバイトコードを生成する */
static void gen_byte_code(CodegenVisitor* visitor, SVM_Opcode op, ...) {
    va_list ap;
    va_start(ap, op);
    
	/* 命令の種類を可変長引数で受け取ったindexから取得 */
    OpcodeInfo oInfo = svm_opcode_info[op];
    printf("-->%s\n", oInfo.opname);
    printf("-->%s\n", oInfo.parameter);
    
	/* メモリのサイズが足りないときはメモリを(+10byte)リサイズする */
    // pos + 1byte + operator (1byte) + operand_size
    if ((visitor->pos + 1 + 1 + (get_opsize(&oInfo))) > visitor->current_code_size) {
        visitor->code = MEM_realloc(visitor->code,
                visitor->current_code_size += visitor->CODE_ALLOC_SIZE);        
    }
    
    visitor->code[visitor->pos++] = op & 0xff;
    
	/* 'i'の数だけint型引数を取る命令と解釈して処理をする */
    for (int i = 0; i < strlen(oInfo.parameter); ++i) {
        switch(oInfo.parameter[i]) {
			/* 引数のある命令 */
            case 'i': { // 2byte index
                int operand = va_arg(ap, int);
				/* 2byteの内上位1byteを格納する */
                visitor->code[visitor->pos++] = (operand >> 8) & 0xff;
				/* 2byteの内下位1byteを格納する */
                visitor->code[visitor->pos++] = operand        & 0xff;                
                break;
            }
            default: {
                fprintf(stderr, "undefined parameter\n");
                exit(1);
                break;
            }
        }
    }
    /*
    for (int i = 0; i < visitor->pos; ++i) {
        printf("%02x ", visitor->code[i]);
    }
    printf("\n");
    */
    va_end(ap);
}

/* 定数プールに定数を1つ追加してインデックスを返す */
static int add_constant(CS_Executable* exec, CS_ConstantPool* cpp) {
    exec->constant_pool = MEM_realloc(exec->constant_pool, 
            sizeof(CS_ConstantPool) * (exec->constant_pool_count+1));
    exec->constant_pool[exec->constant_pool_count] = *cpp;
    return exec->constant_pool_count++;
}


static void enter_castexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter castexpr : %d\n", expr->u.cast_expression.ctype);
}
static void leave_castexpr(Expression* expr, Visitor* visitor) { 
//    fprintf(stderr, "leave castexpr\n");
}

static void enter_boolexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter boolexpr : %d\n", expr->u.boolean_value);
}
static void leave_boolexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave boolexpr\n");
}


static void enter_intexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter intexpr : %d\n", expr->u.int_value);

}

/* intの式をstackにpushするバイトコードをvisitorに渡す */
static void leave_intexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave intexpr\n");
    CS_Executable* exec = ((CodegenVisitor*)visitor)->exec;
    CS_ConstantPool cp;
    cp.type = CS_CONSTANT_INT;
    cp.u.c_int = expr->u.int_value;
    int idx = add_constant(exec, &cp);
    gen_byte_code((CodegenVisitor*)visitor, SVM_PUSH_INT, idx);    
}

static void enter_doubleexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter doubleexpr : %f\n", expr->u.double_value);
}
static void leave_doubleexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave doubleexpr\n");            
}

static void enter_identexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter identifierexpr : %s\n", expr->u.identifier.name);
}

/* 変数や関数の式(参照や代入が行われる)のバイトコードを生成する */
static void leave_identexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave identifierexpr\n");            
    CodegenVisitor* c_visitor = (CodegenVisitor*)visitor;
    switch (c_visitor->v_state) {
		/* 参照 */
        case VISIT_NORMAL: {
//            fprintf(stderr, "push value to stack\n");
            if (expr->u.identifier.is_function) {
//                printf("name=%s, index=%d\n", 
//                        expr->u.identifier.u.function->name,
//                        expr->u.identifier.u.function->index);
                gen_byte_code(c_visitor, SVM_PUSH_FUNCTION,
                        expr->u.identifier.u.function->index);
            } else {
                switch(expr->type->basic_type) {
                    case CS_BOOLEAN_TYPE:
                    case CS_INT_TYPE: {
                        gen_byte_code(c_visitor, SVM_PUSH_STACK_INT,
                                expr->u.identifier.u.declaration->index);
                        break;
                    }
                    case CS_DOUBLE_TYPE: {
                        fprintf(stderr, "double not implementerd visit_nomal in leave_identexpr codegenvisitor\n");
                        exit(1);
                                                
                    }
                    default: {
                        fprintf(stderr, "%d: unknown type in visit_normal in leave_identexpr codegenvisitor\n", expr->line_number); 
                        exit(1);
                    }
                }
            }
            break;
        }
		/* 
		 * 代入
		 * 式の右辺をpushしてから左辺でpopする
		 */
        case VISIT_NOMAL_ASSIGN: {
//            fprintf(stderr, "store value to index\n");
            
            if (!expr->u.identifier.is_function) {
//                fprintf(stderr, "index = %d\n", expr->u.identifier.u.declaration->index);
//                fprintf(stderr, "type = %s\n", get_type_name(expr->type->basic_type));
                switch (expr->type->basic_type) {
                    case CS_BOOLEAN_TYPE:
                    case CS_INT_TYPE:    {
                        gen_byte_code(c_visitor, SVM_POP_STATIC_INT, 
                                expr->u.identifier.u.declaration->index);
                        break;
                    }
                    case CS_DOUBLE_TYPE: {
                        fprintf(stderr, "double not implementerd in leave_identexpr codegenvisitor\n");
                        exit(1);
                    }
                    default: {
                        fprintf(stderr, "unknown type in leave_identexpr codegenvisitor\n");
                        exit(1);
                    }
                }
            } else {
                fprintf(stderr, "%d: cannot assign value to function\n", expr->line_number);
                exit(1);
            }
            
            if (c_visitor->assign_depth > 1) { // nested assign
                switch(expr->type->basic_type) {
                    case CS_BOOLEAN_TYPE:
                    case CS_INT_TYPE: {
                        gen_byte_code(c_visitor, SVM_PUSH_STACK_INT,
                                expr->u.identifier.u.declaration->index);
                        break;
                    }
                    case CS_DOUBLE_TYPE: {
                        fprintf(stderr, "double not implementerd assign_depth in leave_identexpr codegenvisitor\n");
                        exit(1);
                                                
                    }
                    default: {
                        fprintf(stderr, "%d: unknown type in leave_identexpr codegenvisitor\n", expr->line_number); 
                        exit(1);
                    }
                }
            }                                                            
            
            break;
        }
        default: {
            fprintf(stderr, "no such v_state error\n");
            exit(1);
        }
    }
}


static void enter_addexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter addexpr : +\n");
}
static void leave_addexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave addexpr\n");
    switch(expr->type->basic_type) {
        case CS_INT_TYPE: {
            gen_byte_code((CodegenVisitor*)visitor, SVM_ADD_INT);
            break;
        }
        case CS_DOUBLE_TYPE: {
            gen_byte_code((CodegenVisitor*)visitor, SVM_ADD_DOUBLE);
            break;
        }
        default: {
            fprintf(stderr, "%d: unknown type in leave_addexpr codegenvisitor\n", expr->line_number); 
            exit(1);
        }
        
    }
}

static void enter_subexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter subexpr : -\n");
}
static void leave_subexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave subexpr\n");
}

static void enter_mulexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter mulexpr : *\n");
}
static void leave_mulexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave mulexpr\n");
}

static void enter_divexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter divexpr : /\n");
}
static void leave_divexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave divexpr\n");
}

static void enter_modexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter modexpr : mod \n");
}
static void leave_modexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave modexpr\n");
}


static void enter_gtexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter gtexpr : > \n");
}
static void leave_gtexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave gtexpr\n");
}

static void enter_geexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter geexpr : >= \n");
}
static void leave_geexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave geexpr\n");
}

static void enter_ltexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter ltexpr : < \n");
}
static void leave_ltexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave ltexpr\n");
}

static void enter_leexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter leexpr : <= \n");
}
static void leave_leexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave leexpr\n");
}

static void enter_eqexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter eqexpr : == \n");
}
static void leave_eqexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave eqexpr\n");
}

static void enter_neexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter neexpr : != \n");
}
static void leave_neexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave neexpr\n");
}

static void enter_landexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter landexpr : && \n");
}
static void leave_landexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave landexpr\n");
}

static void enter_lorexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter lorexpr : || \n");
}
static void leave_lorexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave lorexpr\n");
}

static void enter_incexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter incexpr : ++ \n");
}
static void leave_incexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave incexpr\n");
}

static void enter_decexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter decexpr : -- \n");
}
static void leave_decexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave decexpr\n");
}

static void enter_minusexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter minusexpr : - \n");
}
static void leave_minusexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave minusexpr\n");
}

static void enter_lognotexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter lognotexpr : ! \n");
}
static void leave_lognotexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave lognotexpr\n");
}

static void enter_assignexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter assignexpr : %d \n", expr->u.assignment_expression.aope);
    ((CodegenVisitor*)visitor)->assign_depth++;
}
static void leave_assignexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave assignexpr\n");
    --((CodegenVisitor*)visitor)->assign_depth;
//    ((CodegenVisitor*)visitor)->v_state = VISIT_NORMAL;
}

static void notify_assignexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "NOTIFY assignexpr : %d \n", expr->u.assignment_expression.aope);
    ((CodegenVisitor*)visitor)->v_state = VISIT_NOMAL_ASSIGN;    
}

static void enter_funccallexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "enter function call :\n");
}
static void leave_funccallexpr(Expression* expr, Visitor* visitor) {
//    fprintf(stderr, "leave function call\n");
    gen_byte_code((CodegenVisitor*)visitor, SVM_INVOKE);
}

/* For statement */
static void enter_exprstmt(Statement* stmt, Visitor* visitor) {
//    fprintf(stderr, "enter exprstmt :\n");

}
static void leave_exprstmt(Statement* stmt, Visitor* visitor) {
//    fprintf(stderr, "leave exprstmt\n");
    
    CodegenVisitor* c_visitor = (CodegenVisitor*)visitor;
    switch (c_visitor->v_state) {
        case VISIT_NORMAL: {
            gen_byte_code(c_visitor, SVM_POP);
            break;
        }
        case VISIT_NOMAL_ASSIGN: {            
            c_visitor->v_state = VISIT_NORMAL;
            c_visitor->assign_depth = 0;
            break;
        }
        default: {
            fprintf(stderr, "no such visit state in leave_exprstmt\n");
            break;
        }
    }
    
//    ((CodegenVisitor*)visitor)->v_state = VISIT_NORMAL;
    
    
}

static void enter_declstmt(Statement* stmt, Visitor* visitor) {
//    fprintf(stderr, "enter declstmt name=%s, type=%s:\n", 
//            stmt->u.declaration_s->name,
//            get_type_name(stmt->u.declaration_s->type->basic_type));
            
}

static void leave_declstmt(Statement* stmt, Visitor* visitor) {
//    fprintf(stderr, "leave declstmt\n");
}

/*
 * CodegenVisitorのメモリを確保して
 * - コンパイラ
 * - バイトコード情報のポインタ
 * - コードサイズ/使用領域
 * - バイトコードポインタ
 * - visit status
 * - 代入の深さ
 * - 関数ポインタ
 * を設定する
 */
CodegenVisitor* create_codegen_visitor(CS_Compiler* compiler, CS_Executable *exec) {
    visit_expr* enter_expr_list;
    visit_expr* leave_expr_list;
    visit_stmt* enter_stmt_list;
    visit_stmt* leave_stmt_list;
    
    visit_expr* notify_expr_list;
    
    if (compiler == NULL || exec == NULL) {
        fprintf(stderr, "Compiler or Executable is NULL\n");
        exit(1);
    }
    
    CodegenVisitor* visitor = (CodegenVisitor*)MEM_malloc(sizeof(CodegenVisitor));
    visitor->compiler = compiler;
    visitor->exec = exec;
    visitor->CODE_ALLOC_SIZE = 10; // temporary
    visitor->current_code_size = 0;
    visitor->pos = 0;
    visitor->code = NULL;
    visitor->v_state = VISIT_NORMAL;
    visitor->assign_depth = 0;
    

    enter_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    leave_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    notify_expr_list = (visit_expr*)MEM_malloc(sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    
    enter_stmt_list = (visit_stmt*)MEM_malloc(sizeof(visit_stmt) * STATEMENT_TYPE_COUNT_PLUS_ONE);
    leave_stmt_list = (visit_stmt*)MEM_malloc(sizeof(visit_stmt) * STATEMENT_TYPE_COUNT_PLUS_ONE);
    
    memset(enter_expr_list, 0, sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    memset(leave_expr_list, 0, sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);
    memset(notify_expr_list, 0, sizeof(visit_expr) * EXPRESSION_KIND_PLUS_ONE);    
    memset(enter_stmt_list, 0, sizeof(visit_expr) * STATEMENT_TYPE_COUNT_PLUS_ONE);
    memset(leave_stmt_list, 0, sizeof(visit_expr) * STATEMENT_TYPE_COUNT_PLUS_ONE);

    
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
    
    notify_expr_list[ASSIGN_EXPRESSION]       = notify_assignexpr;
    
    
    
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

    ((Visitor*)visitor)->notify_expr_list = notify_expr_list;
    
    
    
    
    return visitor;
}
