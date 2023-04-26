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
size_t readn(long fd, void *buf, size_t size) {
    size_t left = size;
    long r = -2;
    char *bufptr = (char*)buf;

    while(left > 0) {
        if( (r=read((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (r == 0)
            return 0;   // EOF
        left    -= r;
        bufptr  += r;
    }
    return size;
}

size_t writen(int fd, void *buf, size_t size) {
    size_t left = size;
    long r;
    void *bufptr = (char*)buf;
    while(left>0) {
        if ((r=write((int)fd ,bufptr,left)) == -1) {
            perror("writen");
            return -1;
        }
        if (r == 0) return 0;
        left    -= r;
        bufptr  += r;
    }
    return 1;
}