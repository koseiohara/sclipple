


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "strutils.h"
#include "edit_list.h"


// return -1 when io error
// return 1 if file is empty or white
// return 0 otherwise
int get_first_line(const char* notename, const int first_line_len, char* first_line){
    FILE* fp;
    char* enter;

    fp = fopen(notename, "r");
    if (fp == NULL){
        #ifdef DEBUG
        printf("<DEBUG> get_first_line() failed to open file\n");
        #endif
        perror(notename);
        return -1;
    }

    #ifdef DEBUG
    printf("<DEBUG> opened %s\n", notename);
    #endif

    while (fgets(first_line, first_line_len, fp) != NULL){
        #ifdef DEBUG
        printf("<DEBUG> [LINE]: %s\n", first_line);
        #endif
        if (is_white_space(first_line)){
            #ifdef DEBUG
            printf("<DEBUG> SKIP\n");
            #endif
            continue;
        }

        enter = strchr(first_line, '\n');
        if  (enter == NULL){
            first_line[first_line_len-4] = '.';
            first_line[first_line_len-3] = '.';
            first_line[first_line_len-2] = '.';
            first_line[first_line_len-1] = '\0';
        } else{
            *enter = '\0';
        }

        fclose(fp);
        return 0;
    }

    #ifdef DEBUG
    printf("<DEBUG> %s is completely empty!\n", notename);
    #endif

    first_line[0] = '\0';
    fclose(fp);
    return 1;
}


int ls(const char* list, int flag_num, char** flag_list){
    struct stat st;
    FILE*  fp;
    char*  flag     = NULL;
    char*  datetime = NULL;
    char*  notename = NULL;
    char** lines    = NULL;        // <flag>:\n  created: <datetime>\n  <first_line>\n\n\0
    char   first_line[LS_LINE_LEN];
    int    result;
    int    i;
    int    j;
    int    len;

    #ifdef DEBUG
    printf("<DEBUG> Number of Flags: %d\n", flag_num);
    for (i = 0; i < flag_num; i = i + 1){
        printf("<DEBUG> Flag %d: %s\n", i, flag_list[i]);
    }
    #endif

    result = path_status(list, &st);
    if (result != 1){
        if (result == 0){
            fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        } else if (result == -1){
            fprintf(stderr, "%s IO Error: Failed to access list file\n", PROGRAM);
        }
        return -1;
    } 

    if (flag_num < 0){
        fprintf(stderr, "%s Error: Invalid number of flags: %d\n", PROGRAM, flag_num);
        return -1;
    } else if (flag_num > 0){
        lines = malloc((size_t)flag_num * sizeof(char*));
        if (lines == NULL){
            perror("malloc");
            return -1;
        }

        for (j = 0; j < flag_num; j = j + 1){
            lines[j] = malloc(sizeof(char));
            lines[j][0] = '\0';
        }
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        for (j = 0; j < flag_num; j = j + 1){
            free(lines[j]);
        }
        free(lines);
        return -1;
    }

    result = 0;
    i      = 0;
    while (1){
        i = i + 1;
        // return 0 if successed
        // return 1 if input failed
        // return 2 if line is white space
        // return -1 if list file is broken: does not have thee elements
        result = get_content_line(fp, &flag, &datetime, &notename);
        if (result == 1){
            break;
        } else if (result == 2){
            continue;
        } else if (result == -1){
            fprintf(stderr, "%s Error: Invalid line is found in list file\nlist file is broken in line %d\n", PROGRAM, i);
            for (j = 0; j < flag_num; j = j + 1){
                free(lines[j]);
            }
            free(lines);
            fclose(fp);
            free(flag);
            free(datetime);
            free(notename);
            return -1;
        }

        #ifdef DEBUG
        printf("<DEBUG> read: FLAG = %s, DATETIME = %s, NOTENAME = %s\n", flag, datetime, notename);
        #endif

        if (flag_num > 0){
            for (j = 0; j < flag_num; j = j + 1){
                if (strcmp(flag_list[j], flag) == 0){
                    if (get_first_line(notename, LS_LINE_LEN, first_line) < 0){
                        for (j = 0; j < flag_num; j = j + 1){
                            free(lines[j]);
                        }
                        free(lines);
                        fclose(fp);
                        free(flag);
                        free(datetime);
                        free(notename);
                        return -1;
                    }
                    len = 28;           // 5 \n, 8 spaces, 3 corons, \0, created, file, and 4 variables
                    len = len + strlen(flag);
                    len = len + strlen(datetime);
                    len = len + strlen(first_line);
                    lines[j] = realloc(lines[j], len*sizeof(char));
                    snprintf(lines[j], len, "%s:\n  created: %s\n  file: %s\n  %s\n\n", flag, datetime, notename, first_line);
                }
            }
        } else{
            if (get_first_line(notename, LS_LINE_LEN, first_line) < 0){
                free(lines);
                fclose(fp);
                free(flag);
                free(datetime);
                free(notename);
                return -1;
            }
            printf("%s:\n  created: %s\n  file: %s\n  %s\n\n", flag, datetime, notename, first_line);
        }

        free(flag);
        free(datetime);
        free(notename);

        flag     = NULL;
        datetime = NULL;
        notename = NULL;
    }

    // free(flag);
    // free(datetime);
    // free(notename);

    if (flag_num > 0){
        for (i = 0; i < flag_num; i = i + 1){
            #ifdef DEBUG
            printf("Note %d: %s\n", i, flag_list[i]);
            #endif
            if (lines[i][0] == '\0'){
                fprintf(stderr, "%s Error: Note '%s' was not found\n", PROGRAM, flag_list[i]);
                for (j = 0; j < flag_num; j = j + 1){
                    free(lines[j]);
                }
                free(lines);
                fclose(fp);
                return -2;
            }
        }

        for (i = 0; i < flag_num; i = i + 1){
            printf("%s", lines[i]);
        }
    }

    for (j = 0; j < flag_num; j = j + 1){
        free(lines[j]);
    }
    free(lines);

    fclose(fp);
    return 0;
}



