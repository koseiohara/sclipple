

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return INVALID_KEY_ERROR if new_flag is invalid
// return IO_ERROR if failed to update list file
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return FILE_FORMAT_ERROR if file name in list file does not include "--"
// return RENAME_ERROR if failed to rename old_file to new_file
// return KEY_NOT_FOUND if old_flag does not exist
// return KEY_DUPLICATE if new_flag already exist
// return UNKNOWN_ERROR if program bug is found
// return 0 otherwise
int mv(const char* list, char* old_flag, char* new_flag){
    struct stat st;
    int result;
    char* new_file = NULL;
    char* old_file = NULL;

    // check new keyword
    result = flag_validation(new_flag);
    if (result < 0){
        if (result == INPUT_ERROR){
            fprintf(stderr, "%s: Keyword is empty\n", PROGRAM);
        } else if (result == CHARACTER_NOT_ALLOWED_ERROR){
            fprintf(stderr, "%s: Invalid character is included in '%s'\nKeywords can include alphabets, numbers, '_', and '-'\n", PROGRAM, new_flag);
        } else if (result == RESERVED_WORD_ERROR){
            fprintf(stderr, "%s: '%s' is a reserved word\n", PROGRAM, new_flag);
        }
        return INVALID_KEY_ERROR;
    }

    // check whether list file is exist
    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PROGRAM);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PROGRAM);
        }
        return IO_ERROR;
    } 

    result = get_filename_by_key(list, old_flag, &old_file);
    if (result < 0){
        if (result == IO_ERROR){
            fprintf(stderr, "%s: Failed to open %s\n", PROGRAM, list);
            free(old_file);
            return IO_ERROR;
        } else if (result == LIST_FORMAT_ERROR || result == INPUT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PROGRAM);
            free(old_file);
            return LIST_FORMAT_ERROR;
        } else if (result == MALLOC_ERROR){
            free(old_file);
            return MALLOC_ERROR;
        }
        fprintf(stderr, "%s: Unknown error\n", PROGRAM);
        free(old_file);
        return UNKNOWN_ERROR;
    } else if (result == KEY_NOT_FOUND){
        fprintf(stderr, "%s: No such key: '%s'\n", PROGRAM, old_flag);
        free(old_file);
        return KEY_NOT_FOUND;
    }

    // rewrite list file
    result = mv_key_in_list(list, old_flag, new_flag);
    if (result < 0){
        free(old_file);
        if (result == IO_ERROR || result == RENAME_ERROR){
            fprintf(stderr, "%s: Failed to update list file\n", PROGRAM);
            return IO_ERROR;
        } else if (result == MALLOC_ERROR){
            return MALLOC_ERROR;
        } else if (result == LIST_FORMAT_ERROR || result == FILE_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PROGRAM);
            return LIST_FORMAT_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PROGRAM);
            return UNKNOWN_ERROR;
        }
    } else if (result == KEY_NOT_FOUND){
        fprintf(stderr, "%s: No such key: '%s'\n", PROGRAM, old_flag);
        free(old_file);
        return KEY_NOT_FOUND;
    } else if (result == KEY_DUPLICATE){
        fprintf(stderr, "%s: New keyword '%s' already exists\n", PROGRAM, new_flag);
        free(old_file);
        return KEY_DUPLICATE;
    }

    // get new file name
    result = mv_filename(old_file, new_flag, &new_file);
    if (result < 0){
        if (result == FILE_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PROGRAM);
            free(new_file);
            free(old_file);
            return FILE_FORMAT_ERROR;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: MALLOC ERROR\n", PROGRAM);
            free(new_file);
            free(old_file);
            return MALLOC_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PROGRAM);
            free(new_file);
            free(old_file);
            return UNKNOWN_ERROR;
        }
    }
    printf("%s: RENAME %s -> %s\n", PROGRAM, old_flag, new_flag);

    #ifdef DEBUG
    printf("<DEBUG> Rename %s to %s\n", old_file, new_file);
    // return 0;
    #endif

    // rename file
    if (rename(old_file, new_file) == 0){
        free(old_file);
        free(new_file);
        return 0;
    }

    perror(new_file);
    free(old_file);
    free(new_file);
    return RENAME_ERROR;
}


