#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "tvm.h"
#include "lexer.h"
#include "compiler.h"
#include "token.h"
#include "value.h"
#include "instruction.h"
#include "object.h"
#include "butil.h"


// Some forward declarations
static ParseRule* getRule(TokenType t);
static void parsePrecedence(Parser *p, Scanner *sc, Compiler* c, Precedence prec);
static void expression(Parser *p, Scanner *sc, Compiler* c);
static void statement(Parser *p, Scanner *sc, Compiler* c);
static void declaration(Parser *p, Scanner *sc, Compiler* c);

Program* compilingProgram;

void initIntervalArray(LiveInterval* array){
    array->capacity = 0;
    array->count = 0;
    array->intervals = NULL;
}

void writeIntervalArray(LiveInterval* array, ido_uint32 r, ido_uint32 start, ido_uint32 end){
    if(array->capacity < array->count + 1){
        int oldCap = array->capacity;
        array->capacity = GROW_CAPACITY(oldCap);
        array->intervals = GROW_ARRAY(Intervals, array->intervals, oldCap, array->capacity);
    }

    Intervals l;
    l.end = end;
    l.start = start;
    l.registerIndex = r;

    array->intervals[array->count] = l;
    array->count++;

}
void freeIntervalArray(LiveInterval* array){
    FREE_ARRAY(LiveInterval, array->intervals, array->capacity);
    initIntervalArray(array);
}

static inline void addLiveInterval(Compiler* c, ido_uint32 r, ido_uint32 start, ido_uint32 end){
    writeIntervalArray(&c->liveIntervals, r, start, end);
}

static int compareStart(const void *a, const void *b){
    Intervals *A = (Intervals *)a;
    Intervals *B = (Intervals *)b;
    return (A->start - B->start);
}

static inline void sortIntervalsByStart(LiveInterval* array){
    qsort(array->intervals, array->count, sizeof(&array->intervals), compareStart);
}






// Some big TODO stuff above



ido_uint32 getLastAllocatedRegister(Compiler* c){
    return c->tvm->last_allocated_register;
}

void setLastAllocatedRegister(Compiler* c, ido_uint32 r){
    c->tvm->last_allocated_register = r;
}


void freeR(Compiler* c, Parser* p, ido_uint32 r){
    if(p->hadError){
        return;
    }

    if(!IS_REGISTER_FREE(r) || !c->tvm->allocatedRegisters[r]){
        fprintf(stderr, "attempted to free unallocated or invalid register R%d\n", r);
        exit(1);
    }

    c->tvm->allocatedRegisters[r] = false;
    c->tvm->free_register_count++;
    printf("free R%d (free: %d)\n", r, c->tvm->free_register_count);
}

ido_uint32 allocR(Compiler* c, Parser* p){
    for(ido_uint32 i = 0; i < REGISTERS_NUM; i++){
        if(!c->tvm->allocatedRegisters[i]){
            c->tvm->allocatedRegisters[i] = true;
            c->tvm->free_register_count--;
            c->tvm->last_allocated_register = i;
            printf("alloc R%d (free: %d) (line %i)\n", i, c->tvm->free_register_count, p->previous.line);
            return i;
        }
    }
    fprintf(stderr, "no free registers\n"); // need to handle spilling later
    exit(1);
}

Compiler* initCompiler(TVM* tvm){
    Compiler* c = ALLOCATESTRUCT(Compiler);
    if(c == NULL){
        fprintf(stderr, "error allocating compiler: not enough memory");
        exit(1);
    }
    c->localCount = 0;
    c->scopeDepth = 0;
    c->tvm = tvm;
    initIntervalArray(&c->liveIntervals);

    return c;
}

void freeCompiler(Compiler* c){
    freeIntervalArray(&c->liveIntervals);
    FREE(Compiler, c);
}

static Program* currentProgram(){
    return compilingProgram;
}

static void errorAt(Parser* p, Token* token, const char* message){
    if(p->panicMode) return;
    p->panicMode = true;

    fprintf(stderr, "[line %d] error", token->line);

    if(token->type == T_EOF){
        fprintf(stderr, " at end");
    } else if(token->type == T_ERROR){

    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    
    p->hadError = true;
}

static void error(Parser* p, const char* message){
    errorAt(p, &p->previous, message);
}

static void errorAtCurrent(Parser* p, const char* message){
    errorAt(p, &p->current, message);
}

static void inline emitReturn(Parser* p){
    writeToProgram(currentProgram(), ENC_RETURN, p->previous.line);
}

static void endCompilation(Parser* p){
    emitReturn(p);
}

static void beginScope(Compiler* c){
    c->scopeDepth++;
}

static void endScope(Parser *p, Compiler* c){
    c->scopeDepth--;
    while(c->localCount > 0 && c->locals[c->localCount - 1].depth > c->scopeDepth){
        int regIndex = c->locals[c->localCount].registerIndex;
        freeR(c, p, regIndex);
        c->localCount--;
    }
}

static void advance(Parser* p, Scanner* sc, Compiler* c){
    p->previous = p->current;
    for(;;){
        p->current = scanToken(sc);
        if(p->current.type != T_ERROR) break;
        errorAtCurrent(p, p->current.start);
    }
}

static void consume(Parser* p, Scanner* sc, Compiler* c, TokenType type, const char* message){
    if(p->current.type == type){
        advance(p, sc, c);
        return;
    }

    errorAtCurrent(p, message);
}

static bool check(Parser* p, Scanner* sc, TokenType t){
    return p->current.type == t;
}

static bool match(Parser* p, Scanner* sc, Compiler* c, TokenType t){
    if(!check(p, sc, t)) return false;
    advance(p, sc, c);
    return true;
}

static ido_uint32 createConstant(Parser* p, Value v){
    ido_uint32 constantIndex = addConstant(currentProgram(), v);
    if(constantIndex > UINT32_MAX){
        error(p, "too many constants in one program");
        return 0;
    }

    return constantIndex;
}

static ido_uint32 emitConstant(Parser *p, Compiler* c, Value v){
    ido_uint32 constantIndex = createConstant(p, v);

    ido_uint32 r = allocR(c, p);
    
    writeToProgram(currentProgram(), ENC_CONSTANT(constantIndex, r), p->previous.line);
    return constantIndex;
}

static int writeJumpIfFalse(Parser *p, Compiler* c){
    writeToProgram(currentProgram(), ENC_JUMP_IF_FALSE(getLastAllocatedRegister(c)), p->previous.line);
    return currentProgram()->count - 1;
}

static int writeJump(Parser *p, Compiler* c){
    writeToProgram(currentProgram(), ENC_JUMP(), p->previous.line);
    return currentProgram()->count - 1;
}

static void patchJump(Parser *p, int offset){

    int jump = currentProgram()->count - offset - 1;

   // ensure the jump doesn't exceed the allowed 18-bit range
    if(jump > 0x3FFFF){ // 0x3FFFF = 18 bits
        error(p, "too much code to jump over");
    }

    currentProgram()->code[offset] = currentProgram()->code[offset] | (jump & 0x3FFFF);

}

static void parsePrecedence(Parser *p, Scanner *sc, Compiler* c, Precedence prec){
    advance(p, sc, c);
    ParseFn prefixRule = getRule(p->previous.type)->prefix;

    if(prefixRule == NULL){
        error(p, "expect expression");
        return;
    }

    bool canAssign = prec  <= PREC_ASSIGNMENT;
    prefixRule(p, sc, c, canAssign);

    while(prec <= getRule(p->current.type)->precedence){
        advance(p, sc, c);
        ParseFn infixRule = getRule(p->previous.type)->infix;
        infixRule(p, sc, c, canAssign);
    }

    if(canAssign && match(p, sc, c, T_EQUAL)){
        error(p, "invalid assignment target");
    }

}

static ido_uint32 identifierConstant(Parser *p, Compiler* c, Token* name){
    return emitConstant(p, c, OBJ_VAL(copyString(c->tvm, name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b){
    if(a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Parser *p, Compiler* c, Token* name){
    for(int i = c->localCount - 1; i >= 0; i--){
        Local* local = &c->locals[i];
        if(identifiersEqual(name, &local->name)){
            if(local->depth == -1){
                error(p, "can't read local variable in its own initializer");
            }
            return local->registerIndex;
        }
    }
    return -1;
}

static void addLocal(Parser *p, Compiler* c, Token name){
    if(c->localCount == LOCALS_NUM){
        error(p, "too many local variables in function");
        return;
    }
    ido_uint32 r = allocR(c, p);
    Local* local = &c->locals[c->localCount++];
    local->name = name;
    local->depth =-1;
    local->registerIndex = r;
}

static void declareVariable(Parser *p, Compiler* c){
    if(c->scopeDepth == 0) return;
    Token* name = &p->previous;
    addLocal(p, c, *name);

}

static ido_uint32 parseVariable(Parser *p, Scanner *sc, Compiler* c, const char* errorMessage){
    consume(p, sc, c, T_IDEN, errorMessage);

    declareVariable(p, c);
    if(c->scopeDepth > 0) return 0;

    ido_uint32 idc = identifierConstant(p, c, &p->previous);

    return idc;
}

static void markInitialized(Parser *p, Compiler* c){
    c->locals[c->localCount - 1].depth = c->scopeDepth;
    writeToProgram(currentProgram(), ENC_SET_LOCAL(getLastAllocatedRegister(c), c->locals[c->localCount - 1].registerIndex), p->previous.line);
}

static void defineVariable(Parser *p, Compiler* c, ido_uint32 global){
    if(c->scopeDepth > 0){
        markInitialized(p, c);
        return;
    }

    writeToProgram(currentProgram(), ENC_DEFINE_GLOBAL(getLastAllocatedRegister(c), global), p->previous.line);

}

static void number(Parser *p, Scanner *sc, Compiler* c, bool canAssign){
    double value = strtod(p->previous.start, NULL);
    Value v = DNUMBER_VAL(value);
    emitConstant(p, c, v);
}

static void string(Parser *p, Scanner *sc, Compiler* c, bool canAssign){ // TODO: Translate stuff like \n here
    emitConstant(p, c, OBJ_VAL(copyString(c->tvm, p->previous.start + 1, p->previous.length - 2)));
}

static void namedVariable(Parser *p, Scanner *sc, Compiler* c, Token name, bool canAssign){
    ido_uint32 arg = resolveLocal(p, c, &name);
    // I know this routine sucks, i will refactor it later. (Ah yes, 'refactor it later'. Sure. Obvioulsly that will happen.)

    if(arg != -1){
        if(canAssign && match(p, sc, c, T_EQUAL)){
            freeR(c, p, getLastAllocatedRegister(c));
            expression(p, sc, c);
            writeToProgram(currentProgram(), ENC_SET_LOCAL(getLastAllocatedRegister(c), arg), p->previous.line);
        } else {

            ido_uint32 resultR = allocR(c, p);
            writeToProgram(currentProgram(), ENC_GET_LOCAL(arg, resultR), p->previous.line);
        }
    } else {
        ido_uint32 arg = identifierConstant(p, c, &name); // TODO: Check bits of encoding and return indexes fo better handling
        if(canAssign && match(p, sc, c, T_EQUAL)){
            freeR(c, p, getLastAllocatedRegister(c));
            expression(p, sc, c);
            writeToProgram(currentProgram(), ENC_SET_GLOBAL(getLastAllocatedRegister(c), arg), p->previous.line);

        } else {
            freeR(c, p, getLastAllocatedRegister(c));
            ido_uint32 resultR = allocR(c, p);
            writeToProgram(currentProgram(), ENC_GET_GLOBAL(arg, resultR), p->previous.line);
        }
    }
    
}

static void variable(Parser *p, Scanner *sc, Compiler* c, bool canAssign){
    namedVariable(p, sc, c, p->previous, canAssign);
}

static void unary(Parser *p, Scanner *sc, Compiler* c, bool canAssign){
    TokenType opType = p->previous.type;
    parsePrecedence(p, sc, c, PREC_UNARY);

    ido_uint32 r = getLastAllocatedRegister(c);

    switch (opType)
    {
    case T_MINUS:
        writeToProgram(currentProgram(), ENC_NEG(r), p->previous.line);
        break;
    case T_BANG:
        writeToProgram(currentProgram(), ENC_NOT(r), p->previous.line);
        break;
    default: return;
    }

}

static void expression(Parser *p, Scanner *sc, Compiler* c){
    parsePrecedence(p, sc, c, PREC_ASSIGNMENT);
}

static void block(Parser *p, Scanner *sc, Compiler* c){
    while(!check(p, sc, T_RIGHT_BRACE) && !check(p, sc, T_EOF)) {
        declaration(p, sc, c);
    }
    
    consume(p, sc, c, T_RIGHT_BRACE, "expect '}' after block");
}

static void varDeclaration(Parser *p, Scanner *sc, Compiler* c){

    ido_uint32 global = parseVariable(p, sc, c, "expect var name"); // Get the constant index of the string name
    freeR(c, p, getLastAllocatedRegister(c));

    if(match(p, sc, c, T_EQUAL)){
        expression(p, sc, c); // its going to set a register to the initial val of the var eg: var a = "test";  the string "test" being the initial val here

    }  else {
        ido_uint32 resultR = allocR(c, p);
        writeToProgram(currentProgram(), ENC_NIL(resultR), p->previous.line); // else it does not have a initial value, allocate a nil instead
        freeR(c, p, getLastAllocatedRegister(c));
    }

    consume(p, sc, c, T_SEMICOLON, "expect ';' after var declaration");
    defineVariable(p, c, global);

}

static void expressionStatement(Parser *p, Scanner *sc, Compiler* c){
    expression(p, sc, c);
    consume(p, sc, c, T_SEMICOLON, "expect ';' after expression");
    freeR(c, p, getLastAllocatedRegister(c)); // maybe>?
}

static void ifStatement(Parser *p, Scanner *sc, Compiler* c){
    consume(p, sc, c, T_LEFT_PAREN, "expect '(' after 'if'");
    expression(p, sc, c);
    consume(p, sc, c, T_RIGHT_PAREN, "expect ')' after condition");

    int thenJump = writeJumpIfFalse(p, c);
    freeR(c, p, getLastAllocatedRegister(c));

    statement(p, sc, c);
    int elseJump = writeJump(p, c);


    patchJump(p, thenJump);

    if(match(p, sc, c, T_ELSE)){
        statement(p, sc, c);
    }

    patchJump(p, elseJump);

}

static void printStatement(Parser *p, Scanner *sc, Compiler* c){
    expression(p, sc, c);
    consume(p, sc, c, T_SEMICOLON, "expect ';' after value");
    writeToProgram(currentProgram(), ENC_PRINT(getLastAllocatedRegister(c)), p->previous.line);
    freeR(c, p, getLastAllocatedRegister(c));
}

static void sync(Parser *p, Scanner *sc, Compiler* c){
    p->panicMode = false;
    
    while(p->current.type != T_EOF){
        if(p->previous.type == T_SEMICOLON) return;
        switch (p->current.type)
        {
        case T_FN:
        case T_VAR:
        case T_FOR:
        case T_IF:
        case T_WHILE:
        case T_PRINT:
        case T_RETURN:
            return;
        
        default:;
        }
        advance(p, sc, c);
    }
}

static void declaration(Parser *p, Scanner *sc, Compiler* c){
    if(match(p, sc, c, T_VAR)){
        varDeclaration(p, sc, c);
    } else {
        statement(p, sc, c);
    }
    if(p->panicMode) sync(p, sc, c);
}

static void statement(Parser *p, Scanner *sc, Compiler* c){
    if(match(p, sc, c, T_PRINT)){
        printStatement(p, sc, c);
    } else if(match(p, sc, c, T_IF)){
        ifStatement(p, sc, c);
    } else if(match(p, sc, c, T_LEFT_BRACE)){
        beginScope(c);
        block(p, sc, c);
        endScope(p, c);
    } else {
        expressionStatement(p, sc, c);
    }

}

static void inline grouping(Parser *p, Scanner *sc, Compiler* c, bool canAssign){
    expression(p, sc, c);
    consume(p, sc, c, T_RIGHT_PAREN, "expect ')' after expression");
}

static void binary(Parser *p, Scanner *sc, Compiler* c, bool canAssign){
    TokenType opType = p->previous.type;
    ParseRule* rule = getRule(opType);

    ido_uint32 leftR = getLastAllocatedRegister(c);
    parsePrecedence(p, sc, c, (Precedence)rule->precedence+1);
    ido_uint32 rightR = getLastAllocatedRegister(c);

    freeR(c, p, leftR);
    freeR(c, p, rightR);
    ido_uint32 resultR = allocR(c, p);

    switch (opType)
    {
    case T_BANG_EQUAL: 
        writeToProgram(currentProgram(), ENC_BANG_EQUAL(resultR, leftR, rightR), p->previous.line);
        break;
    case T_EQUAL_EQUAL: 
        writeToProgram(currentProgram(), ENC_EQUAL(resultR, leftR, rightR), p->previous.line);
        break;
    case T_GREATER: 
        writeToProgram(currentProgram(), ENC_GREATER(resultR, leftR, rightR), p->previous.line);
        break;
    case T_LESS: 
        writeToProgram(currentProgram(), ENC_LESS(resultR, leftR, rightR), p->previous.line);
        break;
    case T_LESS_EQUAL: 
        writeToProgram(currentProgram(), ENC_LESS_EQUAL(resultR, leftR, rightR), p->previous.line);
        break;
    case T_GREATER_EQUAL: 
        writeToProgram(currentProgram(), ENC_GREATER_EQUAL(resultR, leftR, rightR), p->previous.line);
        break;
    case T_PLUS:
        writeToProgram(currentProgram(), ENC_ADD(resultR, leftR, rightR), p->previous.line);
        break;
    case T_MINUS:
        writeToProgram(currentProgram(), ENC_SUB(resultR, leftR, rightR), p->previous.line);
        break;
    case T_STAR:
        writeToProgram(currentProgram(), ENC_MUL(resultR, leftR, rightR), p->previous.line);
        break;
    case T_SLASH:
        writeToProgram(currentProgram(), ENC_DIV(resultR, leftR, rightR), p->previous.line);
        break;
    default: return;
    }

}

static void literal(Parser *p, Scanner *sc, Compiler* c, bool canAssign){
    ido_uint32 resultR = allocR(c, p);

    switch (p->previous.type) {
        case T_TRUE:  writeToProgram(currentProgram(), ENC_TRUE(resultR), p->previous.line); break;
        case T_FALSE: writeToProgram(currentProgram(), ENC_FALSE(resultR), p->previous.line); break;
        case T_NIL:   writeToProgram(currentProgram(), ENC_NIL(resultR), p->previous.line); break;
        default: return;
    }

    freeR(c, p, resultR);

}

ParseRule rules[] = {
  [T_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [T_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [T_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [T_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [T_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [T_DOT]           = {NULL,     NULL,   PREC_NONE},
  [T_MINUS]         = {unary,    binary, PREC_TERM},
  [T_PLUS]          = {NULL,     binary, PREC_TERM},
  [T_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [T_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [T_STAR]          = {NULL,     binary, PREC_FACTOR},
  [T_BANG]          = {unary,    NULL,  PREC_NONE},
  [T_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [T_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [T_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [T_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [T_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [T_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [T_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [T_IDEN]          = {variable, NULL,   PREC_NONE},
  [T_STRING]        = {string,   NULL,   PREC_NONE},
  [T_FLOAT]         = {number,   NULL,   PREC_NONE},
  [T_INT]           = {number,   NULL,   PREC_NONE},
  [T_AND]           = {NULL,     NULL,   PREC_NONE},
  [T_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [T_FALSE]         = {literal,  NULL,   PREC_NONE},
  [T_FOR]           = {NULL,     NULL,   PREC_NONE},
  [T_FN]            = {NULL,     NULL,   PREC_NONE},
  [T_IF]            = {NULL,     NULL,   PREC_NONE},
  [T_NIL]           = {literal,  NULL,   PREC_NONE},
  [T_OR]            = {NULL,     NULL,   PREC_NONE},
  [T_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [T_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [T_TRUE]          = {literal,  NULL,   PREC_NONE},
  [T_VAR]           = {NULL,     NULL,   PREC_NONE},
  [T_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [T_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [T_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static ParseRule* getRule(TokenType t){
    return &rules[t];
}

bool compile(Program* program, Scanner* sc, Parser* p, TVM* tvm){
    Compiler* c = initCompiler(tvm);

    compilingProgram = program;

    p->panicMode = false;
    p->hadError = false;

    advance(p, sc, c);
    while(!match(p, sc, c, T_EOF)){
        declaration(p, sc, c);
    }
    endCompilation(p);

    printBytecodeSimple(compilingProgram);

    freeCompiler(c);

    return !p->hadError;
}
