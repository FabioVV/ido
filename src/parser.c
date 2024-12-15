#include "parser.h"
#include "memory.h"

Parser* initParser(){
    Parser* p = ALLOCATESTRUCT(Parser);
    if(p == NULL) exit(1);
    return p;
}


void freeParser(Parser* p){
    FREE(Parser, p);
}
