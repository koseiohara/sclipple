
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
// #include <strings.h>
#include <string.h>

#include "globals.h"

#define DATETIME_LEN 20
#define PATH_LEN     (DIR_LEN+FILE_LEN+1)


// col is zero-based
// if col=0 is specified, result is not overrided
int read_list_by_key(char* list, char* target_flag, int col, char* result){
    int i;
    FILE* fp;
    char  line[DATETIME_LEN+PATH_LEN+FLAG_LEN+4];
    char* flag;

    fp = fopen(list, "r");

    while(fgets(line, sizeof(line), fp) != NULL){
        flag = strtok(line, ",");
        if (strcmp(flag, target_flag) == 0){
            if (col == 0){
                // result = flag;
                return 0;
            }
            i = 1;
            while (flag != NULL){
                result = strtok(NULL, ",");
                if (i == col){
                    return 0;
                }
                i = i + 1;
            }
            perror("strtok");
            return -1;
        }
    }
    perror("fgets");
    return -1;
}

int flag_exist_check(char* list, char* flag){
    char* dummy;
    return read_list_by_key(list, flag, 0, dummy);
}


int get_datetime_by_key(char* list, char* flag, char* datetime){
    if (read_list_by_key(list, flag, 1, datetime) == -1){
        perror("list");
        return -1;
    }
}


int get_filename_by_key(char* list, char* flag, char* filename){
    if (read_list_by_key(list, flag, 2, filename) == -1){
        perror("list");
        return -1;
    }
}


