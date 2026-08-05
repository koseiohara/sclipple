
#include <stdio.h>

#define XFREE(p)              \
    do {                      \
        free(p);              \
        (p) = NULL;           \
    } while (0)


int xfclose(FILE** fp);

