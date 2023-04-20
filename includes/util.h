#if  !defined(UTIL_H)
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_NAME 256


void* _malloc (unsigned long size);

long isNumber(const char* s);

#endif
