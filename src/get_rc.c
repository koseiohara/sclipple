
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "globals.h"
#include "ptrutils.h"
#include "strutils.h"
#include "file_systems.h"
#include "validation.h"
#include "get_rc.h"


// return MALLOC_ERROR if asprintf or strdup failed
// return 0 otherwise
int init_config(Config* config, char* home){
    int result;

    config->editor = NULL;
    config->ext    = NULL;
    config->dir    = NULL;

    config->editor = strdup("vim -p");
    if (config->editor == NULL){
        return MALLOC_ERROR;
    }

    config->ext = strdup("txt");
    if (config->ext == NULL){
        return MALLOC_ERROR;
    }

    result = asprintf(&(config->dir), "%s/%s", home, DIR);
    if (result < 0){
        return MALLOC_ERROR;
    }
    return 0;
}


void free_config(Config* config){
    XFREE(config->editor);
    XFREE(config->ext);
    XFREE(config->dir);
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

    if (result == MALLOC_ERROR){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        free_config(config);
        return result;
    }

    init_entry(config, entry);
    return result;
}


// return RC_ERROR when bad input
// return FILE_FORMAT_ERROR if directory does not begin with /
// return IO_ERROR when io error
// return MALLOC_ERROR if strdup failed
// return INPUT_ERROR if a bug
// return UNKNOWN_ERROR if an unknown error
// return 0 otherwise
int read_rc(const char* rc, RcEntry* entry, const size_t n_entry){
    FILE*  fp   = NULL;
    char*  line = NULL;
    char*  in_key;
    char*  in_value;
    char*  new_value;
    const char* lbrack = "\"'";
    const char* rbrack = "\"'";
    int    result;
    int    ret = 0;
    size_t i;
    size_t size;

    if (rc == NULL || (entry == NULL && n_entry != 0)){
        fprintf(stderr, "%s: Invalid input to function read_rc()\n", PACKAGE_NAME);
        return INPUT_ERROR;
    }
    for (i = 0; i < n_entry; i = i + 1){
        if (entry[i].key == NULL || entry[i].value == NULL){
            fprintf(stderr, "%s: Invalid input to function read_rc()\n", PACKAGE_NAME);
            return INPUT_ERROR;
        }
    }

    fp = fopen(rc, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, rc, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    size = 0;
    while(getline(&line, &size, fp) != -1){
        // delete comment
        if (line[0] == RC_COMMENT){
            line[0] = '\0';
        }

        if (line_to_dict(line, &in_key, &in_value) != 0){
            continue;
        }

        for (i = 0; i < n_entry; i = i + 1){
            if (strcmp(in_key, entry[i].key) != 0){
                continue;
            }

            new_value = NULL;
            if (strcmp(entry[i].key, "directory") == 0){
                result = parse_directory(in_value, &new_value);
                if (result != 0){
                    if (result == FILE_FORMAT_ERROR){
                        fprintf(stderr, "%s: %s: Invalid directory: '%s'\n"
                                        "Directory must be the absolute path format\n", PACKAGE_NAME, rc, in_value);
                        ret = FILE_FORMAT_ERROR;
                    } else if (result == WORDEXP_ERROR){
                        fprintf(stderr, "%s: %s: Invalid directory: '%s'\n", PACKAGE_NAME, rc, in_value);
                        ret = RC_ERROR;
                    } else if (result == MALLOC_ERROR){
                        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                        ret = MALLOC_ERROR;
                    } else if (result == INPUT_ERROR){
                        fprintf(stderr, "%s: Invalid input to function parse_directory()\n", PACKAGE_NAME);
                        ret = INPUT_ERROR;
                    } else{
                        fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                        ret = UNKNOWN_ERROR;
                    }
                    XFREE(new_value);
                    goto cleanup;
                }
            } else{
                delete_bracket(&in_value, (int)strlen(lbrack), lbrack, rbrack);

                new_value = strdup(in_value);
                if (new_value == NULL){
                    fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                    ret = MALLOC_ERROR;
                    goto cleanup;
                }

                if (strcmp(entry[i].key, "extension") == 0){
                    result = ext_validation(new_value);
                    if (result != 0){
                        if (result == CHARACTER_NOT_ALLOWED_ERROR){
                            fprintf(stderr, "%s: %s: Invalid extension: '%s'\n"
                                            "Extension must consist of alphabets, numbers, '.', '-', and '_'\n"
                                            "Extension cannot start with '.'\n", PACKAGE_NAME, rc, new_value);
                            ret = RC_ERROR;
                        } else{
                            fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                            ret = UNKNOWN_ERROR;
                        }
                        XFREE(new_value);
                        goto cleanup;
                    }
                }
            }

            XFREE(*(entry[i].value));
            *(entry[i].value) = new_value;
            break;
        }
    }

    if (ferror(fp)){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, rc, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }
    ret = 0;
    goto cleanup;


cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, rc, strerror(errno));
            ret = IO_ERROR;
        }
    }
    free(line);

    return ret;
}


int config_update(Config* base, Config new_config){
    if (new_config.dir != NULL){
        XFREE(base->dir);
        base->dir = strdup(new_config.dir);
        if (base->dir == NULL){
            return MALLOC_ERROR;
        }
    }

    if (new_config.ext != NULL){
        XFREE(base->ext);
        base->ext = strdup(new_config.ext);
        if (base->ext == NULL){
            return MALLOC_ERROR;
        }
    }

    if (new_config.editor != NULL){
        XFREE(base->editor);
        base->editor = strdup(new_config.editor);
        if (base->editor == NULL){
            return MALLOC_ERROR;
        }
    }

    return 0;
}


