
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
// #include <strings.h>
#include <string.h>

#include "names.h"
#include "globals.h"

#define DATETIME_LEN 20


// col is zero-based
// if col=0 is specified, result is not overrided
int read_list_by_key(const char* list, char* target_flag, const int col, char* result){
    int i;
    FILE* fp;
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char* flag;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return -2;
    }

    // read line by line
    while(fgets(line, sizeof(line), fp) != NULL){
        // replace '\n' to '\0'
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0'){
            continue;
        }

        // find the first delimiter
        flag = strtok(line, ",");

        #ifdef DEBUG
        printf("<DEBUG> Flag = %s\n", flag);
        #endif
        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            if (col == 0){
                fclose(fp);
                return 0;
            }
            i = 1;
            while (flag != NULL){
                // find target column...
                strcpy(result, strtok(NULL, ","));
                #ifdef DEBUG
                printf("<DEBUG> Col = %d, Word = %s\n", i, result);
                #endif
                if (i == col){
                    #ifdef DEBUG
                    printf("<DEBUG> Extracted Word = %s\n", result);
                    #endif
                    fclose(fp);
                    return 0;
                }
                i = i + 1;
            }
            // if col exceeds the actual number of columns
            fprintf(stderr, "%s: Specified column is too large: %d", flag, 0);
            fclose(fp);
            exit(1);
        }
    }
    // if target_flag is not found
    fclose(fp);
    return -1;
}


// return 0 if flag does not exist
// return -1 if flag exist
int flag_exist_check(const char* list, char* flag){
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


int get_datetime_by_key(const char* list, char* flag, char* datetime){
    int  stat;
    stat = read_list_by_key(list, flag, 1, datetime);

    if (stat < 0){
        perror(list);
        return -1;
    }
    return 0;
}


int get_filename_by_key(const char* list, char* flag, char* filename){
    int  stat;
    stat = read_list_by_key(list, flag, 2, filename);

    if (stat < 0){
        perror(list);
        return -1;
    }
    return 0;
}


int mv_key_in_list(const char* list, const char* old_flag, const char* new_flag){
    FILE* fp;
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char  tmpfile[LIST_APATH_LEN+8];
    char* flag;
    char* datetime;
    char* notename;
    int   fd;

    snprintf(tmpfile, sizeof(tmpfile), "%s.XXXXXX", list);
    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        return -1;
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        close(fd);
        unlink(tmpfile);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL){
        // skip empty line
        if (line[0] == '\0'){
            continue;
        }

        // find the first delimiter
        flag = strtok(line, ",");

        #ifdef DEBUG
        printf("<DEBUG> Flag = %s\n", flag);
        #endif
        // if the flag of the current line is target_flag
        if (strcmp(flag, old_flag) == 0){
            strcpy(datetime, strtok(NULL, ","));
            strcpy(notename, strtok(NULL, ","));
            #ifdef DEBUG
            printf("<DEBUG> FLAG    : %s\n", flag);
            printf("<DEBUG> DATETIME: %s\n", daetime);
            printf("<DEBUG> NOTENAME: %s\n", notename);
            #endif

            mv_filename(new_flag, notename);
            snprintf(line, sizeof(line), "%s,%s,%s\n", new_flag, datetime, notename);
        }
        fprintf(fp, line);
    }

    // if target_flag is not found
    fclose(fp);
    return 0;
}


