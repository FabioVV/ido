#ifndef IDO_IDOCONF
#define IDO_IDOCONF

#include <limits.h>
#include <stddef.h>


/*
    Is true if int has at least 32 bits (Thanks Lua!)
*/
#define IS32INT ((UINT_MAX >> 30) >= 3)


#endif