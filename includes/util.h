#if  !defined(UTIL_H)
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_NAME 256

typedef struct searcher_var {

    char * dir_name;
    int error;

}s_Var;

void* _malloc (unsigned long size);

long isNumber(const char* s);

#endif
