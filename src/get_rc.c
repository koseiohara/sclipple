

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "strutils.h"
#include "names.h"
#include "get_rc.h"


void init_config(Config* config){
    snprintf(config->editor,
             sizeof(config->editor),
             "%s",
             "vim -p"
             );

    snprintf(config->ext,
             sizeof(config->ext),
             "%s",
             "txt"
             );
}


void init_entry(Config* config, RcEntry* entry){
    entry[0].key   = "editor";
    entry[0].value = config->editor;
    entry[0].len   = sizeof(config->editor);

    entry[1].key   = "extension";
    entry[1].value = config->ext;
    entry[1].len   = sizeof(config->ext);
}


void init(Config* config, RcEntry* entry){
    init_config(config);
    init_entry(config, entry);
}


// return -2 when bad input
// return -1 when io error
// return 0 otherwise
int read_rc(const char* rc, RcEntry* entry, const size_t n_entry){
    FILE*  fp;
    char*  line;
    char*  decomm;
    char*  in_key;
    char*  in_value;
    const char* lbrack = "\"'";
    const char* rbrack = "\"'";
    int    i;
    int    n;
    int    result;
    size_t size;

    fp = fopen(rc, "r");
    if (fp == NULL){
        perror(rc);
        return -1;
    }

    line = NULL;
    size = 0;
    n    = (int)n_entry;
    while(getline(&line, &size, fp) != -1){
        // delete comment
        decomm = strchr(line, RC_COMMENT);
        if (decomm != NULL){
            *decomm = '\0';
        }

        if (line_to_dict(line, &in_key, &in_value) < 0){
            continue;
        }

        for (i = 0; i < n; i = i + 1){
            if (strcmp(in_key, entry[i].key) == 0){
                delete_bracket(&in_value, (int)strlen(lbrack), lbrack, rbrack);
                snprintf(entry[i].value, entry[i].len, "%s", in_value);

                if (strcmp(entry[i].key, "extension") == 0){
                    result = ext_validation(entry[i].value);
                    if (result != 0){
                        fprintf(stderr, "%s Invalid extension: %s.\nExtension must consist of alphabets, numbers, '.', '-', and '_'\n", rc, entry[i].value);
                        return -2;
                    }
                }
            }
        }
    }
    fclose(fp);
    free(line);
    return 0;
}

