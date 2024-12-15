#include <stdio.h>
#include "sysinf.h"
#include "ido.h"


void repl(){
    char line[1024];
    int lc = 1;

    TVM* tvm = initVM();
    Scanner* sc = initScanner();
    Parser* p = initParser();

    sys_info_print_repl();
    
    for(;;){
        printf("IDO[%03d]> ", lc);

        if(!fgets(line, sizeof(line), stdin)){
            printf("\n");
            return;
        }

        if(line[0] != '\n' && line[0] != '\0'){
            initScannerSource(sc, line);
            interpret(tvm, sc, p);
        }
        
        lc++;
    }

    freeScanner(sc);
    freeParser(p);
    freeVM(tvm);
}

int main(int argc, const char* argv[]){
    repl();
    return 0;
}
