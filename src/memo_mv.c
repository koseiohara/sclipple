

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return -6 when new key already exist
// return -5 when old key does not exist
// return -4 when list file broken
// return -3 when invalid new flag
// return -2 when rename failed
// return -1 then IO error
// return  0 when successed
int mv(const char* list, char* old_flag, char* new_flag){
    struct stat st;
    int result;
    char new_file[FILE_APATH_LEN];
    char old_file[FILE_APATH_LEN];

    // check new keyword
    result = flag_validation(new_flag);
    if (result < 0){
        if (result == -1){
            fprintf(stderr, "%s Error: Keyword is too long or empty: '%s'. Length should be less than %d\n", PROGRAM, new_flag, FLAG_LEN);
        } else if (result == -2){
            fprintf(stderr, "%s Error: Invalid character is included in '%s'. Keywords can include alphabets, numbers, '_', and '-'\n", PROGRAM, new_flag);
        } else if (result == -3){
            fprintf(stderr, "%s Error: '%s' is a reserved word.\n", PROGRAM, new_flag);
        }
        return -3;
    }

    // check whether list file is exist
    result = path_status(list, &st);
    if (result != 1){
        if (result == 0){
            fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        } else if (result == -1){
            fprintf(stderr, "%s IO Error: Failed to access list file\n", PROGRAM);
        }
        return -1;
    } 

    // get current file name from list file
    result = get_filename_by_key(list, old_flag, sizeof(old_file), old_file);
    if (result != 0){
        fprintf(stderr, "%s Error: '%s': No such key.\n", PROGRAM, old_flag);
        return -5;
    }

    // rewrite list file
    result = mv_key_in_list(list, old_flag, new_flag);
    if (result < 0){
        return -1;
    } else if (result == 1){
        fprintf(stderr, "%s Error: '%s': No such key.\n", PROGRAM, old_flag);
        return -5;
    } else if (result == 2){
        fprintf(stderr, "%s Error: New Keyword '%s' already exists.\n", PROGRAM, new_flag);
        return -6;
    }

    // get new file name
    if (mv_filename(old_file, new_flag, sizeof(new_file), new_file) < 0){
        fprintf(stderr, "%s Error: list file is broken\n", PROGRAM);
        return -4;
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


