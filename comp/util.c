
#include <stdio.h>
#include <string.h>
#include "csua.h"

static CS_Compiler *current_compiler = NULL;

void cs_set_current_compiler(CS_Compiler *compiler) {
    current_compiler = compiler;
}

CS_Compiler* cs_get_current_compiler() {
    return current_compiler;
}

/* 宣言文のリストの末端に要素を追加する */
DeclarationList* cs_chain_declaration(DeclarationList* decl_list, Declaration* decl) {
    DeclarationList* p;    
    DeclarationList* list = cs_create_declaration_list(decl);
    if (decl_list == NULL) return list;
    for (p = decl_list; p->next; p = p->next);
    p->next = list;
    return decl_list;
}

/* 宣言した変数の情報を変数名から取得する */
static Declaration* search_decls_from_list(DeclarationList* list, const char* name) {
    for(; list; list = list->next) {
        if (!strcmp(list->decl->name, name)) {
            return list->decl;
        }
    }
    return NULL;
}
// search from a block temporary
Declaration* cs_search_decl_in_block() {
    return NULL;
}

/* 宣言したグローバル変数の情報を変数名から取得する */
Declaration* cs_search_decl_global(const char* name) {
    CS_Compiler* compiler = cs_get_current_compiler();
    return search_decls_from_list(compiler->decl_list, name);
}
