

#include <stdio.h>
#include <unistd.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


int rename_note(const char* list, char* old_flag, char* new_flag){
    int result;
    char new_file[FILE_APATH_LEN];
    char old_file[FILE_APATH_LEN];

    result = get_filename_by_key(list, old_flag, old_file);
    if (result != 0){
        return -1;
    }

    mv_filename(old_file, new_flag, sizeof(new_file), new_file);

    #ifdef DEBUG
    printf("<DEBUG> Rename %s to %s\n", old_file, new_file);
    return 0;
    #endif

    if (rename(old_file, new_file) == 0){
        return 0;
    }

    perror(new_file);
    return -2;
}


int mv(const char* list, char* old_flag, char* new_flag){
    int result;

    result = rename_note(list, old_flag, new_flag);
    if (result == -1){
        fprintf(stderr, "%s: %s: No such key.\n", PROGRAM, old_flag);
        return -1;
    } else if (result == -2){
        return -1;
    }

    result = mv_key_in_list(list, old_flag, new_flag);
    if (result < 0){
        return -1;
    } else if (result == 1){
        fprintf(stderr, "%s: %s: No such key.\n", PROGRAM, old_flag);
        return 1;
    } else if (result == 2){
        fprintf(stderr, "%s: New Keyword %s already exists.", PROGRAM, new_flag);
        return 2;
    }

    return 0;
}


