
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "strutils.h"
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
int rm(const char* list, const char* dir, const char* subdir, const char* trashdir, const char* trashfile, const char* match, int nkeys, char** keys, int ntags, char** tags){
    struct stat st;
    FILE*      fp           = NULL;
    ListField* field_by_key = NULL;
    ListField* field_by_tag = NULL;
    ListField* field_merged = NULL;
    char**     unfound = NULL;
    char*      tmpdir  = NULL;
    char*      tmpfile = NULL;
    int        found_by_keys;       // number of contents found by searching with keys
    int        found_by_tags;       // number of contents found by searching with tags
    int        nconts;              // total number of contents found by searching with keys and tags
    int        result;
    int        ret = 0;
    int        i;

    if (nkeys < 0 || ntags < 0){
        if (nkeys < 0){
            fprintf(stderr, "%s: Unknown error: No keys were speicified to add\n", PACKAGE_NAME);
        } else{
            fprintf(stderr, "%s: Unknown error: Number of tags is negative\n", PACKAGE_NAME);
        }
        ret = INPUT_ERROR;
        goto cleanup;
    }

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

    result = duplication_filter(&nkeys, keys);
    if (result != 0){
        fprintf(stderr, "%s: Unknown error by dulication_filter()\n", PACKAGE_NAME);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    // result = duplication_filter(&ntags, tags);
    // if (result != 0){
    //     fprintf(stderr, "%s: Unknown error by dulication_filter()\n", PACKAGE_NAME);
    //     ret = UNKNOWN_ERROR;
    //     goto cleanup;
    // }

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    if (nkeys > 0){
        unfound = malloc((size_t)nkeys * sizeof(char*));
        if (unfound == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }
    } else{
        unfound = malloc(sizeof(char*));
        if (unfound == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }
        unfound[0] = NULL;
    }
    result = get_content_by_key_and_tag(fp, subdir, match,
                                        nkeys, keys, &found_by_keys, unfound, 
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

    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
        goto cleanup;
    }

    for (i = 0; i < nkeys && unfound[i] != NULL; i = i + 1){
        fprintf(stderr, "%s: No such key: %s\n", PACKAGE_NAME, unfound[i]);
        ret = KEY_NOT_FOUND;
    }

    if (nconts == 0){
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }

    result = asprintf(&tmpdir, "%s/%s", dir, trashdir);
    if (result < 0){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    result = asprintf(&tmpfile, "%s/%s", tmpdir, trashfile);
    if (result < 0){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    result = make_dir(tmpdir);
    if (result != 0 && result != IS_DIRECTORY){
        if (result == IS_NOT_DIRECTORY_ERROR){
            fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, tmpdir);
        } else if (result == MKDIR_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, tmpdir, strerror(errno));
        }
        ret = IO_ERROR;
        goto cleanup;
    }

    for (i = 0; i < nconts; i = i + 1){
        // check whether old file is accessible
        result = path_status(field_merged[i].file, &st);
        if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access note %s\n", PACKAGE_NAME, field_merged[i].file);
            ret = IO_ERROR;
            continue;
        } 

        if (rename(field_merged[i].file, tmpfile) != 0){
            fprintf(stderr, "%s: %s -> %s: %s\n", PACKAGE_NAME, field_merged[i].file, tmpfile, strerror(errno));
            ret = RENAME_ERROR;
            goto cleanup;
        }

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
            } else if (result == KEY_NOT_FOUND){
                fprintf(stderr, "%s: No such key: %s\n", PACKAGE_NAME, field_merged[i].key);
                ret = KEY_NOT_FOUND;
            } else{
                fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                ret = UNKNOWN_ERROR;
            }

            // rollback
            if (rename(tmpfile, field_merged[i].file) != 0){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, field_merged[i].file, strerror(errno));
                goto cleanup;
            }

            goto cleanup;
        }

        printf("%s: removed '%s'\n", PACKAGE_NAME, field_merged[i].key);
    }

    if (nconts == 1){
        printf("%s: %d key was removed\n"  , PACKAGE_NAME, nconts);
    } else{
        printf("%s: %d keys were removed\n", PACKAGE_NAME, nconts);
    }

    if (unlink(tmpfile) != 0){
        fprintf(stderr, "%s: %s: Failed to remove temporary file: %s\n", PACKAGE_NAME, tmpfile, strerror(errno));
        ret = UNLINK_ERROR;
    }

    goto cleanup;


cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
    }
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
    free(tmpdir);
    free(tmpfile);

    return ret;
}


