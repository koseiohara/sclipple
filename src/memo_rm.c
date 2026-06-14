
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"

int delete_note(const char* list, char* flag){
    int result;
    char filename[FILE_APATH_LEN];

    result = get_filename_by_key(list, flag, filename);
    if (result != 0){
        return -1;
    }

    #ifdef DEBUG
    printf("<DEBUG> Delete %s\n", filename);
    #endif

    if (unlink(filename) == 0){
        return 0;
    }

    perror(flag);
    return -2;
}


int rm(const char* list, char* flag){
    int  result;

    #ifdef DEBUG
    printf("List file name: %s\n", list);
    #endif

    if (1 - path_exist(list)){
        fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        return -1;
    }

    result = delete_note(list, flag);
    if (result == -1){
        fprintf(stderr, "%s Error: %s: No such key.\n", PROGRAM, flag);
        return -1;
    } else if (result == -2){
        return -1;
    }

    result = rm_key_in_list(list, flag);
    if (result < 0){
        return -1;
    } else if (result == 1){
        fprintf(stderr, "%s Error: %s: No such key.\n", PROGRAM, flag);
        return 1;
    }

    printf("%s: remove %s\n", PROGRAM, flag);
    return 0;
}


