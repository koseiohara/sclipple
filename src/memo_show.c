

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


int show_one_file(char* flag, char* file){
    FILE*  fp;
    char*  line = NULL;
    int    atty;
    size_t size = 0;

    fp = fopen(file, "r");
    if (fp == NULL){
        perror(file);
        return -1;
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


// return -1 if IO error
// return 0 if successed
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
    if (result != 1){
        if (result == 0){
            fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        } else if (result == -1){
            fprintf(stderr, "%s IO Error: Failed to access list file\n", PROGRAM);
        }
        return -1;
    } 

    if (flag_num > 0){
        notename_list = malloc((size_t)flag_num * sizeof(char*));
        if (notename_list == NULL){
            perror("malloc");
            return -1;
        }

        for (j = 0; j < flag_num; j = j + 1){
            notename_list[j] = malloc(1);
            if (notename_list[j] == NULL){
                perror("malloc");

                for (i = 0; i < j; i = i + 1){
                    free(notename_list[i]);
                }
                free(notename_list);
                return -1;
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
        return -1;
    }

    i = 0;
    while (1){
        i = i + 1;
        result = get_content_line(fp, &flag, &datetime, &notename);
        if (result == 1){
            break;
        } else if (result == 2){
            continue;
        } else if (result < 0){
            fprintf(stderr, "%s Error: Invalid line is found in list file\nlist file is broken in line %d\n", PROGRAM, i);
            fclose(fp);
            for (j = 0; j < flag_num; j = j + 1){
                free(notename_list[j]);
            }
            free(notename_list);
            free(flag);
            free(datetime);
            free(notename);
            return -1;
        }

        if (flag_num > 0){
            for (j = 0; j <  flag_num; j = j + 1){
                if (strcmp(flag, flag_list[j]) == 0){
                    // snprintf(notename_list[j], FILE_APATH_LEN, "%s", notename);
                    free(notename_list[j]);
                    notename_list[j] = strdup(notename);
                }
            }
        } else{
            if (show_one_file(flag, notename) != 0){
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                free(flag);
                free(notename);
                return -1;
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
                fprintf(stderr, "%s Error: Note '%s' was not found\n", PROGRAM, flag_list[j]);
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                free(flag);
                free(notename);
                return -1;
            }
        }
        for (j = 0; j <  flag_num; j = j + 1){
            if (show_one_file(flag_list[j], notename_list[j]) != 0){
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                fclose(fp);
                return -1;
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



