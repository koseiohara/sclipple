
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "strutils.h"
#include "names.h"
#include "get_rc.h"


// return MALLOC_ERROR if asprintf or strdup failed
// return 0 otherwise
int init_config(Config* config, char* home){
    int result;

    config->editor = strdup("vim -p");
    if (config->editor == NULL){
        perror("strdup");
        return MALLOC_ERROR;
    }

    config->ext = strdup("txt");
    if (config->ext == NULL){
        perror("strdup");
        return MALLOC_ERROR;
    }

    result = asprintf(&(config->dir), "%s/%s", home, DIR);
    if (result < 0){
        perror("asprintf");
        return MALLOC_ERROR;
    }
    return 0;
}


void free_config(Config* config){
    free(config->editor);
    free(config->ext);
    free(config->dir);
}


void init_entry(Config* config, RcEntry* entry){
    entry[0].key   = "editor";
    entry[0].value = &config->editor;
    entry[0].len   = strlen(config->editor);

    entry[1].key   = "extension";
    entry[1].value = &config->ext;
    entry[1].len   = strlen(config->ext);

    entry[2].key   = "directory";
    entry[2].value = &config->dir;
    entry[2].len   = strlen(config->dir);
}


// return MALLOC_ERROR if asprintf or strdup failed in init_config
// return 0 otherwise
int init(Config* config, RcEntry* entry, char* home){
    int result;
    result = init_config(config, home);
    init_entry(config, entry);
    return result;
}


// return RC_ERROR when bad input
// return IO_ERROR when io error
// return MALLOC_ERROR if strdup failed
// return INPUT_ERROR if a bug
// return UNKNOWN_ERROR if an unknown error
// return 0 otherwise
int read_rc(const char* rc, RcEntry* entry, const size_t n_entry){
    FILE*  fp;
    char*  line = NULL;
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
        return IO_ERROR;
    }

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

                free(*(entry[i].value));
                *(entry[i].value) = NULL;
                *(entry[i].value) = strdup(in_value);
                if (*(entry[i].value) == NULL){
                    perror("strdup");
                    return MALLOC_ERROR;
                }

                if (strcmp(entry[i].key, "extension") == 0){
                    result = ext_validation(*(entry[i].value));
                    if (result != 0){
                        fprintf(stderr, "%s: Invalid extension: '%s'\nExtension must consist of alphabets, numbers, '.', '-', and '_'\n", rc, *(entry[i].value));

                        fclose(fp);
                        free(line);
                        return RC_ERROR;
                    }
                } else if (strcmp(entry[i].key, "directory") == 0){
                    free(*(entry[i].value));
                    result = parse_directory(in_value, entry[i].value);
                    if (result >= 0){
                        // printf("Directory: %s\n", *(entry[i].value));
                        continue;
                    } else{
                        if (result == RC_ERROR){
                            fprintf(stderr, "%s: Invalid directory: '%s'\nDirectory must be the absolute path format\n", rc, in_value);
                            return RC_ERROR;
                        } else if (result == WORDEXP_ERROR){
                            fprintf(stderr, "%s: Invalid directory specified: '%s'\n", rc, in_value);
                            return RC_ERROR;
                        } else if (result == MALLOC_ERROR){
                            return MALLOC_ERROR;
                        } else if (result == INPUT_ERROR){
                            fprintf(stderr, "%s: Invalid input to function parse_directory()\n", PACKAGE_NAME);
                            return INPUT_ERROR;
                        } else{
                            fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                            return UNKNOWN_ERROR;
                        }
                    }
                }
            }
        }
    }
    fclose(fp);
    free(line);
    return 0;
}

