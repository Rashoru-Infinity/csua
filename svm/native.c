#include <stdio.h>
#include "../memory/MEM.h"
#include "svm.h"

static SVM_Value native_print(SVM_VirtualMachine* svm, SVM_Value* values, int arg_count) {
    SVM_Value v;
    v.ival = 0;
    printf("hoge\n");
    return v;    
}

/* svmにネイティブ関数を追加する */
void add_native_functions(SVM_VirtualMachine* svm) {
    svm_add_native_function(svm, native_print, "print", 0);
}






