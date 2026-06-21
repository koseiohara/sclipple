

#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "strutils.h"
#include "edit_list.h"


#define NOTE_EMPTY 1

// return IO_ERROR if failed to open note
// return NOTE_EMPTY if note is empty
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
        return IO_ERROR;
    }

    #ifdef DEBUG
    printf("<DEBUG> opened %s\n", notename);
    #endif

    while (fgets(first_line, first_line_len, fp) != NULL){
        #ifdef DEBUG
        printf("<DEBUG> [LINE]: %s\n", first_line);
        #endif
        if (is_white_space(first_line) == true){
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
    return NOTE_EMPTY;
}


// return IO_ERROR if list file does not exist, failed to open list file, or failed to read the first line of note
// return INPUT_ERROR if flag_num is a negative value
// return MALLOC_ERROR if malloc or asprintf failed
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if one or more flags is not found
// return UNKNOWN_ERROR if error handling is not enough
// return 0 otherwise
int ls(const char* list, int flag_num, char** flag_list){
    struct stat st;
    FILE*  fp;
    char*  flag     = NULL;
    char*  datetime = NULL;
    char*  notename = NULL;
    char** lines    = NULL;
    char   first_line[LS_LINE_LEN];
    int    result;
    int    i;
    int    j;

    #ifdef DEBUG
    printf("<DEBUG> Number of Flags: %d\n", flag_num);
    for (i = 0; i < flag_num; i = i + 1){
        printf("<DEBUG> Flag %d: %s\n", i, flag_list[i]);
    }
    #endif

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        }
        return IO_ERROR;
    } 

    if (flag_num < 0){
        fprintf(stderr, "%s: Invalid number of flags: %d\n", PACKAGE_NAME, flag_num);
        return INPUT_ERROR;
    } else if (flag_num > 0){
        lines = malloc((size_t)flag_num * sizeof(char*));
        if (lines == NULL){
            perror("malloc");
            return MALLOC_ERROR;
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
        return IO_ERROR;
    }

    result = 0;
    i      = 0;
    while (1){
        i = i + 1;
        result = get_content_line(fp, &flag, &datetime, &notename);
        if (result == END_OF_FILE){
            break;
        } else if (result == LIST_WHITE_SPACE){
            continue;
        } else if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: Invalid line is found in list file\nlist file is broken in line %d\n", PACKAGE_NAME, i);
            for (j = 0; j < flag_num; j = j + 1){
                free(lines[j]);
            }
            free(lines);
            fclose(fp);
            free(flag);
            free(datetime);
            free(notename);
            return LIST_FORMAT_ERROR;
        }

        #ifdef DEBUG
        printf("<DEBUG> read: FLAG = %s, DATETIME = %s, NOTENAME = %s\n", flag, datetime, notename);
        #endif

        if (flag_num > 0){
            for (j = 0; j < flag_num; j = j + 1){
                if (strcmp(flag_list[j], flag) == 0){
                    result = get_first_line(notename, LS_LINE_LEN, first_line);
                    if (result < 0){
                        for (j = 0; j < flag_num; j = j + 1){
                            free(lines[j]);
                        }
                        free(lines);
                        fclose(fp);
                        free(flag);
                        free(datetime);
                        free(notename);
                        if (result == IO_ERROR){
                            return IO_ERROR;
                        } else{
                            return UNKNOWN_ERROR;
                        }
                    }
                    result = asprintf(&lines[j], "%s:\n  created: %s\n  file   : %s\n  %s\n\n", flag, datetime, notename, first_line);
                    if (result < 0){
                        perror("asprintf");
                        for (j = 0; j < flag_num; j = j + 1){
                            free(lines[j]);
                        }
                        free(lines);
                        fclose(fp);
                        free(flag);
                        free(datetime);
                        free(notename);
                        return MALLOC_ERROR;
                    }
                }
            }
        } else{
            result = get_first_line(notename, LS_LINE_LEN, first_line);
            if (result < 0){
                free(lines);
                fclose(fp);
                free(flag);
                free(datetime);
                free(notename);
                if (result == IO_ERROR){
                    return IO_ERROR;
                } else{
                    return UNKNOWN_ERROR;
                }
            }
            printf("%s:\n  created: %s\n  file   : %s\n  %s\n\n", flag, datetime, notename, first_line);
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
                fprintf(stderr, "%s: No such key: '%s'\n", PACKAGE_NAME, flag_list[i]);
                for (j = 0; j < flag_num; j = j + 1){
                    free(lines[j]);
                }
                free(lines);
                fclose(fp);
                return KEY_NOT_FOUND;
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



