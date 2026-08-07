
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return IO_ERROR if failed to open list file
// return LIST_FORMAT_ERROR if list file is broken
// return MALLOC_ERROR if strdup failed
// return UNKNOWN_ERROR if program include bugs
// return UNLINK_ERROR if unlink failed
// return KEY_NOT_FOUND if flag does not exist
// return 0 otherwise
int rm(const char* list, int nflag, char** flag){
    struct stat st;
    int   result;
    int   ret;
    int   i;
    char* filename = NULL;

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

    ret = 0;

    for (i = 0; i < nflag; i = i + 1){
        // get the target filename from list file
        result = get_filename_by_key(list, flag[i], &filename);
        if (result != 0){
            if (result == KEY_NOT_FOUND){
                fprintf(stderr, "%s: No such key: '%s'\n", PACKAGE_NAME, flag[i]);
                if (ret == 0){
                    ret = KEY_NOT_FOUND;
                }
                continue;
                // goto cleanup;
            } else if (result == LIST_FORMAT_ERROR || result == INPUT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
                goto cleanup;
            } else if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
                ret = IO_ERROR;
                goto cleanup;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
                goto cleanup;
            }
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
            goto cleanup;
        }

        // delete the target flag line from the list file
        result = rm_key_in_list(list, flag[i]);
        if (result != 0){
            if (result == KEY_NOT_FOUND){
                fprintf(stderr, "%s: No such key: '%s'\n", PACKAGE_NAME, flag[i]);
                ret = KEY_NOT_FOUND;
                goto cleanup;
            } else if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
                goto cleanup;
            } else if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
                // fprintf(stderr, "%s: Failed to update list file\n", PACKAGE_NAME);
                ret = IO_ERROR;
                goto cleanup;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
                goto cleanup;
            } else{
                fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                ret = UNKNOWN_ERROR;
                goto cleanup;
            }
        }

        if (unlink(filename) == 0){
            printf("%s: removed '%s'\n", PACKAGE_NAME, flag[i]);
            continue;
        }

        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, filename, strerror(errno));
        ret = UNLINK_ERROR;
        goto cleanup;
    }

    goto cleanup;


cleanup:
    free(filename);

    return ret;
}


