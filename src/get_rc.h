

#include <stdlib.h>
#include "globals.h"

typedef struct{
    char* editor;
    char* ext;
} Config;


typedef struct{
    const char* key;
    char**  value;
    size_t len;
} RcEntry;

#define N_ENTRY 2


void init(Config* config, RcEntry* entry);
void free_config(Config* config);
int read_rc(const char* rc, RcEntry* entry, const size_t n_entry);


