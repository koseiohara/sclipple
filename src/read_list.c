
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
// #include <strings.h>
#include <string.h>

#include "globals.h"

#define DATETIME_LEN 20
#define PATH_LEN     (DIR_LEN+FILE_LEN+1)


// col is zero-based
// if col=0 is specified, result is not overrided
int read_list_by_key(const char* list, const char* target_flag, const int col, char* result){
    int i;
    FILE* fp;
    char  line[DATETIME_LEN+PATH_LEN+FLAG_LEN+4];
    char* flag;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return -2;
    }

    while(fgets(line, sizeof(line), fp) != NULL){
        flag = strtok(line, ",");
        #ifdef DEBUG
        printf("DEBUG: Flag = %s\n", flag);
        #endif
        if (strcmp(flag, target_flag) == 0){
            if (col == 0){
                // result = flag;
                fclose(fp);
                return 0;
            }
            i = 1;
            while (flag != NULL){
                // result = strtok(NULL, ",");
                strcpy(result, strtok(NULL, ","));
                #ifdef DEBUG
                printf("DEBUG: Col = %d, Word = %s\n", i, result);
                #endif
                if (i == col){
                    #ifdef DEBUG
                    printf("DEBUG: Extracted Word = %s\n", result);
                    #endif
                    fclose(fp);
                    return 0;
                }
                i = i + 1;
            }
            fprintf(stderr, "%s: Specified column is too large: %d", flag, 0);
            fclose(fp);
            exit(1);
        }
    }
    // perror(target_flag);
    fclose(fp);
    return -1;
}

int flag_exist_check(const char* list, const char* flag){
    char dummy[128];
    int  stat;
    stat = read_list_by_key(list, flag, 0, dummy);

    if (stat == -1){
        return 0;
    } else if (stat == 0){
        return -1;
    } else if (stat == -2){
        exit(1);
    }
    return -2;
}


int get_datetime_by_key(const char* list, const char* flag, char* datetime){
    int  stat;
    stat = read_list_by_key(list, flag, 1, datetime);

    if (stat < 0){
        perror(list);
        exit(1);
    }
    return 0;
}


int get_filename_by_key(const char* list, const char* flag, char* filename){
    int  stat;
    stat = read_list_by_key(list, flag, 1, filename);

    if (stat < 0){
        perror(list);
        exit(1);
    }
    return 0;
}


