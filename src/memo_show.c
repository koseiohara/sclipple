

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


int show_one_file(char* file){
    FILE*  fp;
    char*  line;
    int    atty;
    size_t size;

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
        printf("\033[34m%s\033[0m\n", file);
    } else{
        printf("%s\n", file);
    }

    while (getline(&line, &size, fp) != -1){
        printf("%s", line);
    }

    printf("\n");
    fclose(fp);
    free(line);
    return 0;
}


int show(char* list, int flag_num, char** flag_list){
    FILE* fp;
    char  flag[FLAG_LEN];
    char  datetime[DATETIME_LEN];
    char  notename[FILE_APATH_LEN];
    char  (*notename_list)[FILE_APATH_LEN] = NULL;
    int   result;
    int   i;
    int   j;

    if (1 - path_exist(list)){
        fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        return -1;
    }

    if (flag_num > 0){
        notename_list = malloc((size_t)flag_num * sizeof(*notename_list));
        if (notename_list == NULL){
            perror("malloc");
            return -1;
        }

        for (j = 0; j < flag_num; j = j + 1){
            notename_list[j][0] = '\0';
        }
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        free(notename_list);
        return -1;
    }

    i = 0;
    while (1){
        i = i + 1;
        result = get_content_line(fp, sizeof(flag), flag, sizeof(datetime), datetime, sizeof(notename), notename);
        if (result == 1){
            break;
        } else if (result == 2){
            continue;
        } else if (result < 0){
            fprintf(stderr, "%s Error: Invalid line is found in list file\nlist file is broken in line %d\n", PROGRAM, i);
            free(notename_list);
            fclose(fp);
            return -1;
        }

        if (flag_num > 0){
            for (j = 0; j <  flag_num; j = j + 1){
                if (strcmp(flag, flag_list[j]) == 0){
                    snprintf(notename_list[j], FILE_APATH_LEN, "%s", notename);
                }
            }
        } else{
            if (show_one_file(notename) != 0){
                free(notename_list);
                fclose(fp);
                return -1;
            }
        }
    }

    if (flag_num > 0){
        for (j = 0; j <  flag_num; j = j + 1){
            if (notename_list[j][0] == '\0'){
                fprintf(stderr, "%s Error: Note '%s' was not found\n", PROGRAM, flag_list[j]);
                free(notename_list);
                fclose(fp);
                return -1;
            }
        }
        for (j = 0; j <  flag_num; j = j + 1){
            if (show_one_file(notename_list[j]) != 0){
                free(notename_list);
                fclose(fp);
                return -1;
            }
        }
    }
    free(notename_list);
    fclose(fp);
    return 0;
}



