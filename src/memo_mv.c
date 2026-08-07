

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "names.h"
#include "edit_list.h"


// return INVALID_KEY_ERROR if new_flag is invalid
// return IO_ERROR if failed to update list file
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return FILE_FORMAT_ERROR if file name is invalid
// return RENAME_ERROR if failed to rename old_file to new_file
// return KEY_NOT_FOUND if old_flag does not exist
// return KEY_DUPLICATE if new_flag already exist
// return UNKNOWN_ERROR if program bug is found
// return 0 otherwise
int mv(const char* list, char* old_flag, char* new_flag){
    struct stat st;
    int result;
    int ret = 0;
    char* new_file = NULL;
    char* old_file = NULL;

    // check new keyword
    result = flag_validation(new_flag);
    if (result != 0){
        if (result == INPUT_ERROR){
            fprintf(stderr, "%s: Keyword is empty\n", PACKAGE_NAME);
        } else if (result == CHARACTER_NOT_ALLOWED_ERROR){
            fprintf(stderr, "%s: Invalid character is included in '%s'\nKeywords can include alphabets, numbers, '_', and '-'\n", PACKAGE_NAME, new_flag);
        } else if (result == RESERVED_WORD_ERROR){
            fprintf(stderr, "%s: '%s' is a reserved word\n", PACKAGE_NAME, new_flag);
        }
        ret = INVALID_KEY_ERROR;
        goto cleanup;
    }

    // check whether list file is exist
    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        }
        ret = IO_ERROR;
        goto cleanup;
    } 

    result = get_filename_by_key(list, old_flag, &old_file);
    if (result != 0){
        if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
            goto cleanup;
        } else if (result == LIST_FORMAT_ERROR || result == INPUT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        } else if (result == KEY_NOT_FOUND){
            fprintf(stderr, "%s: No such key: '%s'\n", PACKAGE_NAME, old_flag);
            ret = KEY_NOT_FOUND;
            goto cleanup;
        }
        fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    result = flag_exist_check(list, new_flag);
    if (result != false){
        if (result == true){
            fprintf(stderr, "%s: New keyword '%s' already exists\n", PACKAGE_NAME, new_flag);
            ret = KEY_DUPLICATE;
        } else if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
        } else if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    // get new file name
    result = mv_filename(old_file, new_flag, &new_file);
    if (result != 0){
        if (result == FILE_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = FILE_FORMAT_ERROR;
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

    if (link(old_file, new_file) != 0){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, new_file, strerror(errno));
        ret = RENAME_ERROR;
        goto cleanup;
    }

    // rewrite list file
    result = mv_key_in_list(list, old_flag, new_flag, new_file);
    if (result != 0){
        if (result == IO_ERROR || result == RENAME_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
        } else if (result == FILE_FORMAT_ERROR){
            fprintf(stderr, "%s: Invalid filename format: %s\n", PACKAGE_NAME, old_file);
            ret = LIST_FORMAT_ERROR;
        } else if (result == KEY_NOT_FOUND){
            fprintf(stderr, "%s: No such key: '%s'\n", PACKAGE_NAME, old_flag);
            ret = KEY_NOT_FOUND;
        } else if (result == KEY_DUPLICATE){
            fprintf(stderr, "%s: New keyword '%s' already exists\n", PACKAGE_NAME, new_flag);
            ret = KEY_DUPLICATE;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        if (unlink(new_file) != 0){
            fprintf(stderr, "%s: rollback failed: %s: %s\n", PACKAGE_NAME, new_file, strerror(errno));
        }
        goto cleanup;
    }

    if (unlink(old_file) != 0){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, new_file, strerror(errno));
        ret = RENAME_ERROR;
        goto cleanup;
    }

    #ifdef DEBUG
    printf("<DEBUG> Rename %s to %s\n", old_file, new_file);
    // return 0;
    #endif

    printf("%s: RENAME %s -> %s\n", PACKAGE_NAME, old_flag, new_flag);
    ret = 0;
    goto cleanup;


cleanup:
    free(new_file);
    free(old_file);

    return ret;
}


