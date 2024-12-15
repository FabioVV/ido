#include <stdio.h>
#include "sysinf.h"
#include "ido.h"


static void repl(){
    char line[1024];
    int lc = 1;

    TVM* tvm = initVM();
    Parser* p = initParser();

    sys_info_print_repl();
    
    for(;;){
        printf("IDO[%03d]> ", lc);

        if(!fgets(line, sizeof(line), stdin)){
            printf("\n");
            return;
        }

        if(line[0] != '\n' && line[0] != '\0'){
            Scanner* sc = initScanner(line);
            interpret(tvm, sc, p);
            freeScanner(sc);
        }
        
        lc++;
    }

    freeParser(p);
    freeVM(tvm);
}

static char* readFile(const char* path){
    FILE* file = fopen(path, "rb");
    if(file == NULL){
        fprintf(stderr, "Could not open file \"%s\"\n", path);
        exit(74);
    }

    int fs = fseek(file, 0L, SEEK_END);
    if(fs != 0){
        fprintf(stderr, "Could not open file (fssek operation failed) \"%s\".\n", path);
        exit(74);
    }

    size_t fileSize = ftell(file);

    int rewind = fseek(file, 0L, SEEK_SET);
    if(rewind != 0){
        fprintf(stderr, "Could not open file (fssek rewind operation failed) \"%s\".\n", path);
        exit(74);
    }

    char* buffer = (char*)malloc(fileSize+1);
    if(buffer == NULL){
        fprintf(stderr, "Not enough memory to read \"%s\"\n", path);
        exit(74);
    }

    size_t bytedRead = fread(buffer, sizeof(char), fileSize, file);
    if(bytedRead < fileSize){
        fprintf(stderr, "Could not read file \"%s\"\n", path);
        exit(74);
    }

    buffer[bytedRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path){
    TVM* tvm = initVM();
    Parser* p = initParser();
    char* source = readFile(path);
    Scanner* sc = initScanner(source);

    InterpretResult result = interpret(tvm, sc, p);
    free(source);
    
    printf("Free registers after program: %i\n", tvm->free_register_count);

    if(result == INTERPRET_COMPILE_ERROR) exit(65);
    if(result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]){
    if(argc == 1){
        repl();
    } else if(argc == 2){
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: ido [path]\n");
        exit(64);
    }
    return 0;
}
