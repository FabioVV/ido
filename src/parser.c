#include "parser.h"
#include "memory.h"

Parser* initParser(){
    Parser* p = ALLOCATESTRUCT(Parser);
     if(p == NULL){
        fprintf(stderr, "error allocating parser: not enough memory");
        exit(1);
    }
    return p;
}


void freeParser(Parser* p){
    FREE(Parser, p);
}
