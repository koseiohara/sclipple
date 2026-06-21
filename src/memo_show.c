

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return IO_ERROR if failed to open note
// return 0 otherwise
int show_one_file(char* flag, char* file){
    FILE*  fp;
    char*  line = NULL;
    int    atty;
    size_t size = 0;

    fp = fopen(file, "r");
    if (fp == NULL){
        perror(file);
        return IO_ERROR;
    }

    size = 0;
    line = NULL;

    // show file name
    atty = isatty(fileno(stdout));
    if (atty){
        printf("[\033[34m%s\033[0m]\n", flag);
    } else{
        printf("[%s]\n", flag);
    }

    while (getline(&line, &size, fp) != -1){
        printf("%s", line);
    }

    printf("\n");
    fclose(fp);
    free(line);
    return 0;
}


// retutn IO_ERROR if failed to open list file or note
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR if program has a bug
// return KEY_NOT_FOUND if one or more flags do not exist
// return 0 otherwise
int show(char* list, int flag_num, char** flag_list){
    struct stat st;
    FILE*  fp;
    char*  flag     = NULL;
    char*  datetime = NULL;
    char*  notename = NULL;
    char** notename_list = NULL;
    int    result;
    int    i;
    int    j;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access %s\n", PACKAGE_NAME, list);
        }
        return IO_ERROR;
    } 

    if (flag_num > 0){
        notename_list = malloc((size_t)flag_num * sizeof(char*));
        if (notename_list == NULL){
            perror("malloc");
            return MALLOC_ERROR;
        }

        for (j = 0; j < flag_num; j = j + 1){
            notename_list[j] = malloc(1);
            if (notename_list[j] == NULL){
                perror("malloc");

                for (i = 0; i < j; i = i + 1){
                    free(notename_list[i]);
                }
                free(notename_list);
                return MALLOC_ERROR;
            }

            notename_list[j][0] = '\0';
        }
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        for (j = 0; j < flag_num; j = j + 1){
            free(notename_list[j]);
        }
        free(notename_list);
        return IO_ERROR;
    }

    i = 0;
    while (1){
        i = i + 1;
        result = get_content_line(fp, &flag, &datetime, &notename);
        if (result == END_OF_FILE){
            break;
        } else if (result == LIST_WHITE_SPACE){
            continue;
        } else if (result < 0){
            fclose(fp);
            for (j = 0; j < flag_num; j = j + 1){
                free(notename_list[j]);
            }
            free(notename_list);
            free(flag);
            free(datetime);
            free(notename);
            if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                return LIST_FORMAT_ERROR;
            }
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            return UNKNOWN_ERROR;
        }

        if (flag_num > 0){
            for (j = 0; j <  flag_num; j = j + 1){
                if (strcmp(flag, flag_list[j]) == 0){
                    free(notename_list[j]);
                    notename_list[j] = strdup(notename);
                }
            }
        } else{
            if (show_one_file(flag, notename) == IO_ERROR){
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                free(flag);
                free(notename);
                return IO_ERROR;
            }
        }
        free(flag);
        free(datetime);
        free(notename);

        flag     = NULL;
        datetime = NULL;
        notename = NULL;
    }

    if (flag_num > 0){
        for (j = 0; j <  flag_num; j = j + 1){
            if (notename_list[j][0] == '\0'){
                fprintf(stderr, "%s: No such note: '%s'\n", PACKAGE_NAME, flag_list[j]);
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                return KEY_NOT_FOUND;
            }
        }
        for (j = 0; j <  flag_num; j = j + 1){
            result = show_one_file(flag_list[j], notename_list[j]);
            if (result != 0){
                fclose(fp);
                if (result == IO_ERROR){
                    for (j = 0; j < flag_num; j = j + 1){
                        free(notename_list[j]);
                    }
                    free(notename_list);
                    fprintf(stderr, "%s: Failed to open %s\n", PACKAGE_NAME, notename_list[j]);
                    return IO_ERROR;
                }
                fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                return UNKNOWN_ERROR;
            }
        }
    }
    for (j = 0; j < flag_num; j = j + 1){
        free(notename_list[j]);
    }
    free(notename_list);
    fclose(fp);
    return 0;
}



