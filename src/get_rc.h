

#include <stdlib.h>
#include "globals.h"

typedef struct{
    char editor[EDITOR_LEN];
    char ext[EXT_LEN];
} Config;


typedef struct{
    const char* key;
    char*  value;
    size_t len;
} RcEntry;

#define N_ENTRY 2


void init(Config* config, RcEntry* entry);
int read_rc(const char* rc, RcEntry* entry, const size_t n_entry);


