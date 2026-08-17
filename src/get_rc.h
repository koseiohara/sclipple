
#ifndef _GET_RC_H
#define _GET_RC_H

#include <stdlib.h>
#include "globals.h"

typedef struct{
    char* editor;
    char* ext;
    char* dir;
    char* tag_match;
} Config;


typedef struct{
    const char* key;
    char**  value;
    size_t len;
} RcEntry;

#define N_ENTRY 4


int init(Config* config, RcEntry* entry, char* home);
void free_config(Config* config);
int read_rc(const char* rc, RcEntry* entry, const size_t n_entry);
int config_update(Config* base, Config new_config);

#endif

