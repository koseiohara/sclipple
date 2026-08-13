
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "globals.h"
#include "ptrutils.h"
#include "strutils.h"
#include "validation.h"
#include "list_utils.h"
#include "memo_tag.h"


// return INPUT_ERROR if an argument is invalid
// return INVALID_KEY_ERROR if keys are invalid
// return INVALID_TAG_ERROR if tags are invalid
// return UNKNOWN_ERROR if a bug is found
// return MALLOC_ERROR if malloc failed
// return IO_ERROR if IO failed
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if one or more keys do not exist
// return 0 otherwise
int tag(const char* list, const char* mode, int nkeys, char** keys, int ntags, char** tags){
    const int mode_tag  = 1;
    const int mode_utag = 2;
    FILE* fp = NULL;

    char** exists  = NULL;
    char** nexists = NULL;
    char*  keyline = NULL;
    char*  tagline = NULL;
    int result;
    int ret = 0;
    int exist_count;
    int imode;
    int i;

    if (strcmp(mode, "tag") == 0){
        imode = mode_tag;
    } else if (strcmp(mode, "utag") == 0){
        imode = mode_utag;
    } else{
        fprintf(stderr, "%s: Unknown error: Invalid mode in tag()\n", PACKAGE_NAME);
        ret = INPUT_ERROR;
        goto cleanup;
    }

    if (nkeys <= 0 || ntags < 0){
        if (nkeys <= 0){
            fprintf(stderr, "%s: Unknown error: No keys were speicified to add\n", PACKAGE_NAME);
        } else{
            fprintf(stderr, "%s: Unknown error: Number of tags is negative\n", PACKAGE_NAME);
        }
        ret = INPUT_ERROR;
        goto cleanup;
    }

    // Tag validation
    for (i = 0; i < ntags; i = i + 1){
        if (tags[i] == NULL){
            fprintf(stderr, "%s: Unknown Error: tags[%d] is NULL\n", PACKAGE_NAME, i);
            ret = INPUT_ERROR;
            goto cleanup;
        }

        result = tag_validation(tags[i]);
        if (result != 0){
            if (result == INPUT_ERROR){
                fprintf(stderr, "%s: Tag is empty\n", PACKAGE_NAME);
            } else if (result == CHARACTER_NOT_ALLOWED_ERROR){
                fprintf(stderr, "%s: Invalid character is included in a tag '%s'. Tags can include alphabets, numbers, '_', '-', and '.'\n", PACKAGE_NAME, tags[i]);
            } else if (result == RESERVED_WORD_ERROR){
                fprintf(stderr, "%s: '%s' is a reserved word.\n", PACKAGE_NAME, tags[i]);
            }
            ret = INVALID_TAG_ERROR;
            goto cleanup;
        }
    }
    if (tags[ntags] != NULL){
        fprintf(stderr, "%s: Unknown Error: tags[%d] is not NULL\n", PACKAGE_NAME, ntags);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    result = duplication_filter(&nkeys, keys);
    if (result != 0){
        fprintf(stderr, "%s: Unknown error by dulication_filter()\n", PACKAGE_NAME);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    result = duplication_filter(&ntags, tags);
    if (result != 0){
        fprintf(stderr, "%s: Unknown error by dulication_filter()\n", PACKAGE_NAME);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    exists  = malloc((size_t)nkeys * sizeof(char*));
    nexists = malloc((size_t)nkeys * sizeof(char*));
    if (exists == NULL || nexists == NULL){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    result = key_exist_check(fp, nkeys, keys, exists, nexists);
    if (result == KEY_NOT_FOUND || result == 0){
        for (i = 0; i < nkeys && nexists[i] != NULL; i = i + 1){
            fprintf(stderr, "%s: No such key: %s\n", PACKAGE_NAME, nexists[i]);
        }
        // i = 0;
        // while (exists[i] != NULL){
        //     fprintf(stderr, "%s: Key '%s' already exists\n", PACKAGE_NAME, exists[i]);
        //     i = i + 1;
        // }
        ret = KEY_NOT_FOUND;
        if (exists[0] == NULL){
            goto cleanup;
        }
    } else{
        if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    exist_count = 0;
    if (exists[nkeys-1] != NULL){
        exist_count = nkeys;
    } else{
        for (i = 0; i < nkeys && exists[i] != NULL; i = i + 1){
            exist_count = exist_count + 1;
        }
    }

    result = arr2line(exist_count, exists, &keyline);
    if (result != 0){
        if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else{
            fprintf(stderr, "Unknown error: arr2line\n");
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }
    result = arr2line(ntags, tags, &tagline);
    if (result != 0){
        if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else{
            fprintf(stderr, "Unknown error: arr2line\n");
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    if (imode == mode_tag){
        result = edit_list(list, "tag" , exist_count, exists, tags);
    } else if (imode == mode_utag){
        result = edit_list(list, "utag", exist_count, exists, tags);
    }

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
        // } else if (result == KEY_DUPLICATE){
        //     fprintf(stderr, "%s: Unknown error. Duplication of a key is found in edit_list() while mode is 'rm'\n", PACKAGE_NAME);
        //     ret = KEY_DUPLICATE;
        } else if (result == KEY_NOT_FOUND){
            fprintf(stderr, "%s: Unknown error: One or more keys was not found by edit_list\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    if (imode == mode_tag){
        printf("%s: New tags %s were added to %s\n", PACKAGE_NAME, tagline, keyline);
    } else if (imode == mode_utag){
        printf("%s: Tags %s were removed from %s\n", PACKAGE_NAME, tagline, keyline);
    }

    goto cleanup;


cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
    }

    free(exists);
    free(nexists);
    free(keyline);
    free(tagline);
    return ret;
}


