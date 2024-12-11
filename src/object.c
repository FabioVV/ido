#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "object.h"
#include "value.h"
#include "tvm.h"
#include "table.h"

#define ALLOCATE_OBJ(tvm, type, objectType)  \
    (type*)allocateObj(tvm, sizeof(type), objectType) \

#define HASH(key, length) hash(key, (uint32_t )length, 0)

static uint32_t hash(const char* key, uint32_t length, uint32_t seed){
    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;
    uint32_t r1 = 15;
    uint32_t r2 = 13;
    uint32_t m = 5;
    uint32_t n = 0xe6546b64;
    uint32_t h = 0;
    uint32_t k = 0;
    uint8_t *d = (uint8_t *) key; // 32 bit extract from `key'
    const uint32_t *chunks = NULL;
    const uint8_t *tail = NULL; // tail - last 8 bytes
    int i = 0;
    int l = length / 4; // chunk length

    h = seed;

    chunks = (const uint32_t *) (d + l * 4); // body
    tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'

    // for each 4 byte chunk of `key'
    for (i = -l; i != 0; ++i) {
        // next 4 byte chunk of `key'
        k = chunks[i];

        // encode next 4 byte chunk of `key'
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        // append to hash
        h ^= k;
        h = (h << r2) | (h >> (32 - r2));
        h = h * m + n;
    }

    k = 0;

    // remainder
    switch (length & 3) { // `len % 4'
        case 3: k ^= (tail[2] << 16);
        case 2: k ^= (tail[1] << 8);

        case 1:
        k ^= tail[0];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        h ^= k;
    }

    h ^= length;

    h ^= (h >> 16);
    h *= 0x85ebca6b;
    h ^= (h >> 13);
    h *= 0xc2b2ae35;
    h ^= (h >> 16);

    return h;

}

static Obj* allocateObj(TVM* tvm, size_t size, ObjType type){
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = tvm->objects;
    tvm->objects = object;
    
    return object;
}

static ObjString* allocateString(TVM* tvm, char* chars, int length, uint32_t hash){
    ObjString* string = ALLOCATE_OBJ(tvm, ObjString, OBJ_STRING);
    string->chars = chars;
    string->length = length;
    string->hash = hash;
    tableSet(&tvm->strings, string, NIL_VAL());
    return string;
}

ObjString* takeString(TVM* tvm, char* chars, int length){
    uint32_t h = HASH(chars, length);
    ObjString* interned = tableFindString(&tvm->strings, chars, length, h);

    if(interned != NULL){
        FREE_ARRAY(char, chars, length+1);
        return interned;
    }
    return allocateString(tvm, chars, length, h);
}

ObjString* copyString(TVM* tvm, const char* chars, int length){

    uint32_t h = HASH(chars, length);

    ObjString* interned = tableFindString(&tvm->strings, chars, length, h);
                printf("teste");

    if(interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length+1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(tvm, heapChars, length, h);
}

void printObject(Value v){
    switch (OBJ_TYPE(v))
    {
    case OBJ_STRING:
        printf("%s\n", AS_CSTRING(v));
        break;
    default: return;
    }
}