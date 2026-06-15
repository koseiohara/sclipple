

#include <stdio.h>
#include <unistd.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


int mv(const char* list, char* old_flag, char* new_flag){
    int result;
    char new_file[FILE_APATH_LEN];
    char old_file[FILE_APATH_LEN];

    if (1 - path_exist(list)){
        fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        return -1;
    }

    // get current file name from list file
    result = get_filename_by_key(list, old_flag, old_file);
    if (result != 0){
        return -1;
    }

    // rewrite list file
    result = mv_key_in_list(list, old_flag, new_flag);
    if (result < 0){
        return -1;
    } else if (result == 1){
        fprintf(stderr, "%s Error: %s: No such key.\n", PROGRAM, old_flag);
        return 1;
    } else if (result == 2){
        fprintf(stderr, "%s Error: New Keyword %s already exists.\n", PROGRAM, new_flag);
        return 2;
    }

    // get new file name
    if (mv_filename(old_file, new_flag, sizeof(new_file), new_file) < 0){
        fprintf(stderr, "%s Error: list file is broken\n", PROGRAM);
    }
    printf("%s: RENAME %s -> %s\n", PROGRAM, old_flag, new_flag);

    #ifdef DEBUG
    printf("<DEBUG> Rename %s to %s\n", old_file, new_file);
    // return 0;
    #endif

    // rename file
    if (rename(old_file, new_file) == 0){
        return 0;
    }

    perror(new_file);
    return -2;
}


