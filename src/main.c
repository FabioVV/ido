#include <stdio.h>
#include "tvm.h"
#include "sysinf.h"

void repl(TVM* tvm){
    char line[1024];
    int lc = 1;
    sys_info_print_repl();
    
    for(;;){
        printf("[%03d]> ", lc);

        if(!fgets(line, sizeof(line), stdin)){
            printf("\n");
            return;
        }

        if(line[0] != '\n' && line[0] != '\0'){
            interpret(tvm, line);
        }
        
        lc++;
    }
}

int main(int argc, const char* argv[]){
    TVM tvm;
    initVM(&tvm);
    repl(&tvm);
    return 0;
}
