#include <stdio.h>
#include "tvm.h"
#include "instruction.h"

void repl(){
    char line[1024];
    int lc = 1;

    for(;;){
        printf("[%03d]> ", lc);

        if(!fgets(line, sizeof(line), stdin)){
            printf("\n");
            return;
        }

        if(line[0] != '\n' && line[0] != '\0'){
            
        }
        lc++;
    }
}

int main(int argc, const char* argv[]){

    TVM* vm = malloc(sizeof(TVM));
    initVM(vm);
    writeToProgram(&vm->program, 0, 1);
    runVM(vm);

    freeVM(vm);
    return 0;
}