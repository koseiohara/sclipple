
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return IO_ERROR if failed to open list file
// return LIST_FORMAT_ERROR if list file is broken
// return MALLOC_ERROR if strdup failed
// return KEY_NOT_FOUND if flag does not exist
// return UNKNOWN_ERROR if program include bugs
// return UNLINK_ERROR if unlink failed
// return 0 otherwise
int rm(const char* list, char* flag){
    struct stat st;
    int   result;
    char* filename = NULL;

    #ifdef DEBUG
    printf("List file name: %s\n", list);
    #endif

    // chack whether list file is exist
    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PROGRAM);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access %s\n", PROGRAM, list);
        }
        return IO_ERROR;
    } 

    // get the target filename from list file
    result = get_filename_by_key(list, flag, &filename);
    if (result < 0){
        if (result == IO_ERROR){
            fprintf(stderr, "%s: Failed to open %s\n", PROGRAM, list);
            free(filename);
            return IO_ERROR;
        } else if (result == LIST_FORMAT_ERROR || result == INPUT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PROGRAM);
            free(filename);
            return LIST_FORMAT_ERROR;
        } else if (result == MALLOC_ERROR){
            free(filename);
            return MALLOC_ERROR;
        }
        fprintf(stderr, "%s: Unknown error\n", PROGRAM);
        free(filename);
        return UNKNOWN_ERROR;
    } else if (result == KEY_NOT_FOUND){
        fprintf(stderr, "%s: No such key: '%s'\n", PROGRAM, flag);
        free(filename);
        return KEY_NOT_FOUND;
    }

    // delete the target flag line from the list file
    result = rm_key_in_list(list, flag);
    if (result < 0){
        if (result == IO_ERROR){
            fprintf(stderr, "%s: Failed to update list file\n", PROGRAM);
            free(filename);
            return IO_ERROR;
        } else if (result == MALLOC_ERROR){
            free(filename);
            return MALLOC_ERROR;
        } else if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PROGRAM);
            free(filename);
            return LIST_FORMAT_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PROGRAM);
            free(filename);
            return UNKNOWN_ERROR;
        }
    } else if (result == KEY_NOT_FOUND){
        fprintf(stderr, "%s: No such key: '%s'\n", PROGRAM, flag);
        free(filename);
        return KEY_NOT_FOUND;
    }

    if (unlink(filename) == 0){
        printf("%s: removed '%s'\n", PROGRAM, flag);
        free(filename);
        return 0;
    }

    perror(flag);
    free(filename);
    return UNLINK_ERROR;
}


