

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
    #endif

    if (rename(old_file, new_file) == 0){
        return 0;
    }

    perror(new_file);
    return -2;
}


int mv(){
    flag_exist_check(new_flag)
}


