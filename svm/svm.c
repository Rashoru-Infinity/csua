#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "svm.h"
#include "../memory/MEM.h"

/* 渡したポインタから4byte分値を取りだしてintの値を返す 取り出した分だけポインタをずらす */
static int read_int(uint8_t **p) {
    uint8_t v1 = **p; (*p)++;
    uint8_t v2 = **p; (*p)++;
    uint8_t v3 = **p; (*p)++;
    uint8_t v4 = **p; (*p)++;        
    int v =(int)( v1 << 24 | v2 << 16 | v3 << 8 | v4);
    return v;
}

/* 渡したポインタから8byte分値を取りだしてdoubleの値を返す 取り出した分だけポインタをずらす */
static double read_double(uint8_t **p) {
    uint8_t v1 = **p; (*p)++;
    uint8_t v2 = **p; (*p)++;
    uint8_t v3 = **p; (*p)++;
    uint8_t v4 = **p; (*p)++;
    uint8_t v5 = **p; (*p)++;
    uint8_t v6 = **p; (*p)++;
    uint8_t v7 = **p; (*p)++;
    uint8_t v8 = **p; (*p)++;
    
    long lv = (long)((long)v1 << 56 | (long)v2 << 48 | (long)v3 << 40 | (long)v4 << 32
			| (long)v5 << 24 | (long)v6 << 16 | (long)v7 << 8 | (long)v8);    
    double* pdv = (double*)&lv;
    return *pdv;

}

/* 渡したポインタから1byte分値を取りだして符号なしの1byte分の値を返す 取り出した分だけポインタをずらす */
static uint8_t read_byte(uint8_t **p) {
    uint8_t v = **p; (*p)++;
    return v;
}

/* ファイル先頭8byte(header)の内容をchar型として出力する 読み出した分だけポインタをずらす */
static void parse_header(uint8_t **p) {
    for(int i = 0; i < 8; ++i) {
        printf("%c ", (char)(**p));
        (*p)++;
    }
    printf("\n");        
}

typedef struct {
    char s_buf[128];
    char row_buf[32];    
    int s_index;
    int r_index;
    
} DInfo;

static void dump(DInfo *info) {
    fprintf(stderr, "%s", info->s_buf);
    fprintf(stderr, "   %s\n", info->row_buf);
    info->s_index = 0;
    info->r_index = 0;
}

/* 命令の名前をバッファに突っ込む(20文字) 余ったら空白文字で埋める */
static void add_opname(DInfo *info, const char* str) {
    int len = strlen(str);
    strncpy(&info->s_buf[info->s_index], str, len);
    info->s_index += len;
    for (int i = 0; i < (20-len); ++i, ++info->s_index) {
        info->s_buf[info->s_index] = ' ';
    }        
    info->s_buf[info->s_index] = 0;
}

/* 4文字分空白文字でバッファを埋める */
static void add_padding(DInfo *info) {
    for(int i = 0; i < 4; ++i, ++info->s_index) info->s_buf[info->s_index] = ' ';    
}

/* 2byte分の領域を16進にしてバッファに詰める */
static void add_uint16(DInfo *info, const uint16_t iv) {
    char buf[6] = {};
    sprintf(buf, "%04x", iv);
    strncpy(&info->s_buf[info->s_index], buf, 4);
}

/* 16進ダンプ用のバッファにopの値を16進で突っ込む */
static void add_rowcode(DInfo *info, uint8_t op) {
//    printf("add row %02x\n", op);
    sprintf(&info->row_buf[info->r_index], "%02x", op);
    info->r_index += 2;
    info->row_buf[info->r_index] = 0;
}


static void disasm(SVM_VirtualMachine* svm) {
//static void disasm(uint8_t *code, size_t size) {
    uint8_t *p = svm->code;
    DInfo dinfo = {0};
    int param_len = 0;
    
    for (int i = 0; i < svm->code_size; ++i, p++) {
        OpcodeInfo *oinfo = &svm_opcode_info[*p];
        add_rowcode(&dinfo, *p);
        switch(*p) {
            case SVM_PUSH_INT:
            case SVM_POP_STATIC_INT: 
            case SVM_PUSH_STATIC_INT:
            case SVM_PUSH_FUNCTION:
            case SVM_POP:
            case SVM_ADD_INT:
            case SVM_INVOKE: {
//                printf("%s\n", oinfo->opname);
                add_opname(&dinfo, oinfo->opname);
                break;
            }
            default: {
                fprintf(stderr, "unknown opcode [%02x] in disasm\n", *p);
                exit(1);
            }
        }
		/* 命令の取るパラメータのバイトコードを16進にしてバッファに突っ込む */
        param_len = strlen(oinfo->parameter);
        for (int j = 0; j < param_len; ++j) {
            switch(oinfo->parameter[j]) {
                case 'i': {
                    uint8_t uv = *(++p);
                    uint16_t op = (uint16_t)( uv << 8 | *(++p));
					/* 引数2byte分の16進ダンプをバッファに突っ込む */
                    add_uint16(&dinfo, op);
					/* 先ほど命令の16進ダンプを突っ込んだバッファに引数2byte分の16進ダンプを突っ込む */
                    add_rowcode(&dinfo, uv);
                    add_rowcode(&dinfo, *p);                     
                    i += 2;
                    break;
                }
                default: {
                    fprintf(stderr, "unknown opcode [%c] in disasm\n", oinfo->parameter[j]);
                    exit(1);
                }
            }
        }
		/* 4文字分空白文字でバッファを埋める */
        if (param_len == 0) add_padding(&dinfo);
        /* 命令と引数とコードの16進ダンプを吐き出す */
        dump(&dinfo);        
    }    
}


static void parse(uint8_t* buf, SVM_VirtualMachine* svm) {
    uint8_t* pos = buf;
    parse_header(&pos);
	/* 定数プール内の定数の個数(4byte領域)を読み出す 読み出しただけポインタをずらす */
    svm->constant_pool_count = read_int(&pos);
    printf("constant_pool_count = %d\n", svm->constant_pool_count);
	/* 定数プールのメモリを確保する */
    svm->constant_pool = (SVM_Constant*)MEM_malloc(sizeof(SVM_Constant) * svm->constant_pool_count);
    
    uint8_t type;
	/* 定数の数だけ値を読み出す 読み出しただけポインタをずらす */
    for (int i = 0; i < svm->constant_pool_count; ++i) {
		/* 1byte領域読んだ時の値でintかdoubleか判定する */
        switch(type = read_byte(&pos)) {
			/* intは4byte読み出して共用体にintの値を入れる 読み出した分ポインタをずらす */
            case SVM_INT: {
                int v = read_int(&pos);
                svm->constant_pool[i].type = SVM_INT;
                svm->constant_pool[i].u.c_int = v;
                break;
            }
			/* doubleは8byte読み出して共用体にdoubleの値を入れる 読み出した分だけポインタをずらす */
            case SVM_DOUBLE: {
                double dv = read_double(&pos);
                svm->constant_pool[i].type = SVM_DOUBLE;
                svm->constant_pool[i].u.c_double = dv;                
                break;
            }
            default: {
                fprintf(stderr, "undefined constant type\n in parse");
                exit(1);
            }
        }
    }
    /* 変数の個数を読み出す(4byte) 読み出した分だけポインタをずらす */
    svm->global_variable_count = read_int(&pos);
	/* 変数の値を保持するためのメモリを確保する */
    svm->global_variables = (SVM_Value*)MEM_malloc(sizeof(SVM_Value) * svm->global_variable_count);
	/* 変数の型情報を格納するためのメモリを確保する */
    svm->global_variable_types = (uint8_t*)MEM_malloc(sizeof(uint8_t) * svm->global_variable_count);
    printf("global_variable_count = %d\n", svm->global_variable_count);
    for (int i = 0; i < svm->global_variable_count; ++i) {        
//        uint8_t type = read_byte(&pos);
		/* 変数の型情報を読み出す 読み出した分だけポインタをずらす */
        svm->global_variable_types[i] = read_byte(&pos);
        switch (svm->global_variable_types[i]) {
            case SVM_INT: {
                printf("INT\n");
                break;
            }
            case SVM_DOUBLE: {
                printf("DOUBLE\n");                
                break;
            }
            defulat: {
                break;
            }
        }
    }
    /* コードサイズを読み出す 読み出した分だけポインタをずらす */
    svm->code_size = read_int(&pos);
    printf("code_size = %d\n", svm->code_size);
	/* コード領域のメモリを確保する */
    svm->code = (uint8_t*)MEM_malloc(svm->code_size);
	/* 現在のポインタの位置がコード領域の開始地点なのでそこからコードサイズ分メモリにコピーする */
    memcpy(svm->code, pos, svm->code_size);
	/* メモリにコピーした分(code size分)だけポインタをずらす */
    pos += svm->code_size;
	/* プログラムが使用するスタックサイズを読み出す ポインタをずらす */
    svm->stack_size = read_int(&pos);
    printf("stack_size = %d\n", svm->stack_size);    
    
//    svm->code = pos;
 //   disasm(pos, svm->code_size);
 
}

/* svmのためのメモリを確保して0埋めしてからアドレスを返す */
static SVM_VirtualMachine* svm_create() {
    SVM_VirtualMachine* svm = (SVM_VirtualMachine*)MEM_malloc(sizeof(SVM_VirtualMachine));
    svm->constant_pool = NULL;
    svm->global_variables = NULL;
    svm->global_variable_types = NULL;
    svm->code = NULL;
    svm->constant_pool_count = 0;
    svm->global_variable_count = 0;
    svm->code_size = 0;
    svm->function_count = 0;
    svm->functions = NULL;
    svm->stack = NULL;
    svm->stack_size = 0;
    svm->stack_value_type = NULL;
    svm->pc = 0;
    svm->sp = 0;
    return svm;
}

static void svm_delete(SVM_VirtualMachine* svm) {
    if (!svm) return;
    if (svm->code) {
        MEM_free(svm->code);
    }
    if (svm->constant_pool) {
        MEM_free(svm->constant_pool);
    }
    if (svm->global_variables) {
        MEM_free(svm->global_variables);
    }
    if (svm->global_variable_types) {
        MEM_free(svm->global_variable_types);
    }
    if (svm->functions) {
        MEM_free(svm->functions);
    }
    if (svm->stack) {
        MEM_free(svm->stack);
    }
    if (svm->stack_value_type) {
        MEM_free(svm->stack_value_type);
    }
    
    MEM_free(svm);
}

/* ネイティブ関数のポインタ，関数名，引数の個数を格納する */
void svm_add_native_function(SVM_VirtualMachine* svm, SVM_NativeFunction native_f, char* name, int arg_count) {
    svm->functions = (SVM_Function*)MEM_realloc(svm->functions, sizeof(SVM_Function) * (svm->function_count+1));
    svm->functions[svm->function_count].f_type = NATIVE_FUNCTION;
    svm->functions[svm->function_count].name = name;
    svm->functions[svm->function_count].arg_count = arg_count;
    svm->functions[svm->function_count].u.n_func = native_f;
    svm->function_count++;    
}

/* プログラムカウンタに対応するバイトコードを1byte返す */
static uint8_t fetch(SVM_VirtualMachine* svm) {
    return svm->code[svm->pc++];
}

/* プログラムカウンタに対応するバイトコードを2byte返す */
static uint16_t fetch2(SVM_VirtualMachine* svm) {
    uint8_t v1 = fetch(svm);
    return (v1 << 8) | fetch(svm);
}

static SVM_Constant* read_static(SVM_VirtualMachine* svm, uint16_t idx) {
    return &svm->constant_pool[idx];
}

static int read_static_int(SVM_VirtualMachine* svm, uint16_t idx) {
    return read_static(svm, idx)->u.c_int;
}

static void push_i(SVM_VirtualMachine* svm, int iv) {
    svm->stack[svm->sp].ival = iv;
    svm->stack_value_type[svm->sp] = SVM_INT;
    svm->sp++;
}

static int pop_i(SVM_VirtualMachine *svm) {
    --svm->sp;
    return svm->stack[svm->sp].ival;
}

static void write_i(SVM_Value* head, uint32_t offset, uint32_t idx, int iv) {
    head[offset+idx].ival = iv;
}
static int read_i(SVM_Value* head, uint32_t offset, uint32_t idx) {
    return head[offset+idx].ival;
}

static void write_global_i(SVM_VirtualMachine* svm, uint32_t idx, int iv) {
    write_i(svm->global_variables, 0, idx, iv);
}
static int read_global_i(SVM_VirtualMachine* svm, uint32_t idx) {
    return read_i(svm->global_variables, 0, idx);
}


static void init_svm(SVM_VirtualMachine* svm) {
	/* スタックを必要なサイズだけ確保する */
    svm->stack = (SVM_Value*)MEM_malloc(sizeof(SVM_Value) * svm->stack_size);
	/* 型情報を格納する領域を確保する */
    svm->stack_value_type = (uint8_t*)MEM_malloc(sizeof(uint8_t) * svm->stack_size);
	/* プログラムカウンタを0にする */
    svm->pc = 0;
	/* スタックポインタを0にする */
    svm->sp = 0;
    
	/* グローバル変数をすべて0で初期化する */ 
    for (int i = 0; i < svm->global_variable_count; ++i) {
        switch(svm->global_variable_types[i]) {
            case SVM_INT: {
                svm->global_variables[i].ival = 0;
                break;
            }
            case SVM_DOUBLE: {
                svm->global_variables[i].dval = 0.0;
                break;
            }
            default: {
                fprintf(stderr, "no such svm type\n");
                exit(1);
            }
        }
    }    
}

/* グローバル変数とスタックの内容を出力する */
static void show_status(SVM_VirtualMachine* svm) {
    printf("< show SVM status >\n");
    printf("-- global variable ---\n");
    for (int i = 0; i < svm->global_variable_count; ++i) {

        switch(svm->global_variable_types[i]) {
            case SVM_INT: {
                printf("[%d:svm_int] = %d\n", i, svm->global_variables[i].ival);
                break;
            }
            case SVM_DOUBLE: {
                printf("[%d:svm_dbl] = %f\n", i, svm->global_variables[i].dval);
                break;
            }
            default: {
                fprintf(stderr, "no such svm type\n");
                exit(1);
            }
        }
    }
    printf("\n--- stack ---\n");
    for (int i = (svm->sp - 1); i >= 0; --i) {
        switch(svm->stack_value_type[i]) {
            case SVM_INT: {
                printf("[%d:svm_int] = %d\n", i, svm->stack[i].ival);
                break;
            }
            case SVM_DOUBLE: {
                printf("[%d:svm_dbl] = %f\n", i, svm->stack[i].dval);
                break;
            }
            default: {
                fprintf(stderr, "no such svm type\n");
                exit(1);
            }
        }
    }
    
}

/* スタックマシンを実行する */
static void svm_run(SVM_VirtualMachine* svm) {    
    bool running = true;

    uint8_t op = 0;
    while(running) {
        switch (op = fetch(svm)) {
			/* int型定数をconstant poolから読み出してstackにpush */
            case SVM_PUSH_INT: { // push from constant pool
                uint16_t s_idx = fetch2(svm);
                int v = read_static_int(svm, s_idx);
                push_i(svm, v);
                break;
            }
			/* int型の値をpopしてglobal変数に書き込む */
            case SVM_POP_STATIC_INT: { // save val to global variable
                uint16_t s_idx = fetch2(svm); 
                int iv = pop_i(svm);
                write_global_i(svm, s_idx, iv);
//                show_status(svm);
//                exit(1);
                break;
            }
			/* int型のグローバル変数の値をstackにpushする */
            case SVM_PUSH_STATIC_INT: {
                uint16_t s_idx = fetch2(svm);
                int iv = read_global_i(svm, s_idx);
//                printf("iv = %d\n", iv);
                push_i(svm, iv);
                break;
            }
			/* スタックの値を2つpopして値を足したものをstackにpushし直す */
            case SVM_ADD_INT: {
                int iv1 = pop_i(svm);
                int iv2 = pop_i(svm);
                push_i(svm, (iv1+iv2));
                break;
            }
			/* pushする関数のインデックスを取得する */
            case SVM_PUSH_FUNCTION: {
                uint16_t idx = fetch2(svm);
                push_i(svm, idx);
                break;
            }
			/* 関数を実行する */
            case SVM_INVOKE: {
                uint16_t f_idx = pop_i(svm);
                /*
                printf("function id = %d\n", f_idx);
                printf("function count = %d\n", svm->function_count);
                printf("name = %s\n", svm->functions[f_idx].name);
                printf("sp = %d\n", svm->sp);
                printf("arg_count = %d\n", svm->functions[f_idx].arg_count);
                 */
				/* インデックスに対応する関数を実行する */
                switch (svm->functions[f_idx].f_type) {
                    case NATIVE_FUNCTION: {
                        SVM_Value val = svm->functions[f_idx].u.n_func(svm, 
                                &svm->stack[svm->sp], 
                                svm->functions[f_idx].arg_count);
                        svm->sp -= svm->functions[f_idx].arg_count;
                        svm->stack[svm->sp++] = val;                        
                        break;
                    }
                    default: {
                        fprintf(stderr, "no such function type in invoke\n");
                        exit(1);
                    }
                }


                break;
            }
            case SVM_POP: {
//                printf("sp = %d\n", svm->sp);
                pop_i(svm);
//                exit(1);
                break;
            }
            default: {
                fprintf(stderr, "unknown opcode: %02x in svm_run\n", op);
                show_status(svm);                
                exit(1);                
            }
        }
                              
        running = svm->pc < svm->code_size;
    }
    show_status(svm);

}




int main(int argc, char *argv[]) {

    // for test
    bool disasm_mode = false;
    int file_idx = 1;
    if (argc < 2) {
        fprintf(stderr, "Usage ./svm [optino] file\n");
        exit(1);
    }
    
    if (argc == 3) {
        if (!strcmp("-d", argv[1])) {
          printf("disasm\n");  
          file_idx++;
          disasm_mode = true;                  
        } else {
            fprintf(stderr, "No such option)\n");
        }
    }
    
    SVM_VirtualMachine* svm = svm_create();
    struct stat st;
    if (stat(argv[file_idx], &st) < 0) {
		fprintf(stderr, "%s\n",strerror(errno));
		exit(errno);
	}
    uint8_t* buf = (uint8_t*)malloc(st.st_size);
    int fp = open(argv[file_idx], O_RDONLY); 
    read(fp, buf, st.st_size);
    parse(buf, svm);        
    close(fp);
    free(buf);
    

    
    if (disasm_mode) {
        disasm(svm);
    } else {        
        add_native_functions(svm);        
        init_svm(svm);
        svm_run(svm);
    }
    
    svm_delete(svm);
    MEM_dump_memory();
    
    
    return 0;
}
