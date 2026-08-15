
#ifndef _PTRUTILS_H
#define _PTRUTILS_H

#include <stdio.h>

#define XFREE(p)              \
    do {                      \
        free(p);              \
        (p) = NULL;           \
    } while (0)


int xfclose(FILE** fp);

#endif

