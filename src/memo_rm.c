
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


int rm(const char* list, char* flag){
    struct stat st;
    int  result;
    char filename[FILE_APATH_LEN];

    #ifdef DEBUG
    printf("List file name: %s\n", list);
    #endif

    // chack whether list file is exist
    result = path_status(list, &st);
    if (result != 1){
        if (result == 0){
            fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        } else if (result == -1){
            fprintf(stderr, "%s IO Error: Failed to access list file\n", PROGRAM);
        }
        return -1;
    } 

    // get the target filename from list file
    result = get_filename_by_key(list, flag, sizeof(filename), filename);
    if (result != 0){
        fprintf(stderr, "%s Error: %s: No such key.\n", PROGRAM, flag);
        return -1;
    }

    // delete the target flag line from the list file
    result = rm_key_in_list(list, flag);
    if (result < 0){
        return -1;
    } else if (result == 1){
        fprintf(stderr, "%s Error: %s: No such key.\n", PROGRAM, flag);
        return 1;
    }

    if (unlink(filename) == 0){
        printf("%s: remove %s\n", PROGRAM, flag);
        return 0;
    }

    perror(flag);
    return -2;
}


