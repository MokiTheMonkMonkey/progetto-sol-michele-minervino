#include <util.h>
#include "../includes/util.h"


void* _malloc (unsigned long size) {

    void *elem = NULL;

    if ((elem = malloc(size)) == NULL && size != 0){

        perror("malloc :");
        exit(EXIT_FAILURE);

    }

    return elem;

}

long isNumber(const char* s) {

    char* e = NULL;
    long val = strtol(s, &e, 0);
    if (e != NULL && *e == (char)0) return val;
    return -1;

}
