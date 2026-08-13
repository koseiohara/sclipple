
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "file_systems.h"
#include "list_utils.h"
#include "memo_rm.h"


// return IO_ERROR if failed to open list file
// return LIST_FORMAT_ERROR if list file is broken
// return MALLOC_ERROR if strdup failed
// return UNKNOWN_ERROR if program include bugs
// return UNLINK_ERROR if unlink failed
// return KEY_NOT_FOUND if flag does not exist
// return 0 otherwise
int rm(const char* list, int nkeys, char** keys, int ntags, char** tags){
    struct stat st;
    FILE*      fp           = NULL;
    ListField* field_by_key = NULL;
    ListField* field_by_tag = NULL;
    ListField* field_merged = NULL;
    char**     unfound  = NULL;
    int        found_by_keys;       // number of contents found by searching with keys
    int        found_by_tags;       // number of contents found by searching with tags
    int        nconts;              // total number of contents found by searching with keys and tags
    int        result;
    int        ret = 0;
    int        i;

    #ifdef DEBUG
    printf("List file name: %s\n", list);
    #endif

    // chack whether list file is exist
    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access %s\n", PACKAGE_NAME, list);
        }
        ret = IO_ERROR;
        goto cleanup;
    } 

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }
    unfound = malloc((size_t)nkeys * sizeof(char*));
    result = get_content_by_key_and_tag(fp, nkeys, keys, &found_by_keys, unfound, 
                                        ntags, tags, &found_by_tags, 
                                        &nconts, &field_merged, &field_by_key, &field_by_tag);
    if (result != 0){
        if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
        } else if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    xfclose(&fp);

    for (i = 0; unfound[i] != NULL && i < nkeys; i = i + 1){
        fprintf(stderr, "%s: No such key: %s\n", PACKAGE_NAME, keys[i]);
        ret = KEY_NOT_FOUND;
    }

    if (nconts == 0){
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }


    for (i = 0; i < nconts; i = i + 1){
        result = edit_list(list, "rm", 1, &(field_merged[i].key), NULL);
        if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
            } else if (result == IO_ERROR || result == RENAME_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
                ret = IO_ERROR;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
            } else if (result == KEY_DUPLICATE){
                fprintf(stderr, "%s: Unknown error. Duplication of a key is found in edit_list() while mode is 'rm'\n", PACKAGE_NAME);
                ret = KEY_DUPLICATE;
            } else if (result == KEY_NOT_FOUND){
                fprintf(stderr, "%s: No such key: %s\n", PACKAGE_NAME, field_merged[i].key);
                ret = KEY_DUPLICATE;
            } else{
                fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }

        if (unlink(field_merged[i].file) == 0){
            printf("%s: removed '%s'\n", PACKAGE_NAME, field_merged[i].file);
            continue;
        }

        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, field_merged[i].file, strerror(errno));
        ret = UNLINK_ERROR;
        goto cleanup;
    }

    goto cleanup;


cleanup:
    xfclose(&fp);
    if (field_by_key != NULL){
        for (i = 0; i < nkeys; i = i + 1){
            free_ListField(&(field_by_key[i]));
        }
    }
    if (field_by_tag != NULL){
        for (i = 0; i < found_by_tags; i = i + 1){
            free_ListField(&(field_by_tag[i]));
        }
    }
    free(field_by_key);
    free(field_by_tag);
    free(field_merged);
    free(unfound);

    return ret;
}


