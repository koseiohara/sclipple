

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "names.h"
#include "edit_list.h"


// return IO_ERROR if failed to open note
// return 0 otherwise
int show_one_file(char* flag, char* file){
    FILE*  fp = NULL;
    char*  line = NULL;
    int    atty;
    int    ret;
    size_t size = 0;

    fp = fopen(file, "r");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
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

    if (ferror(fp)){
        ret = IO_ERROR;
        goto cleanup;
    }

    // printf("\n");
    ret = 0;
    goto cleanup;


cleanup:
    xfclose(&fp);
    free(line);

    return ret;
}


// retutn IO_ERROR if failed to open list file or note
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR if program has a bug
// return KEY_NOT_FOUND if one or more flags do not exist
// return 0 otherwise
int show(char* list, int flag_num, char** flag_list){
    struct stat st;
    FILE*  fp = NULL;
    char*  flag     = NULL;
    char*  datetime = NULL;
    char*  notename = NULL;
    char** notename_list = NULL;
    int    result;
    int    ret = 0;
    int    first_echo;
    int    i;
    int    j;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access %s\n", PACKAGE_NAME, list);
        }
        ret = IO_ERROR;
        goto cleanup;
    } 

    if (flag_num > 0){
        notename_list = malloc((size_t)flag_num * sizeof(char*));
        if (notename_list == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            return MALLOC_ERROR;
        }

        for (j = 0; j < flag_num; j = j + 1){
            notename_list[j] = NULL;
        }
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    i = 0;
    while (1){
        result = get_content_line(fp, &flag, &datetime, &notename);
        if (result == END_OF_FILE){
            break;
        } else if (result == LIST_WHITE_SPACE){
            continue;
        } else if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
                goto cleanup;
                // return LIST_FORMAT_ERROR;
            } else if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
                ret = IO_ERROR;
                goto cleanup;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
                goto cleanup;
            }
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
            goto cleanup;
        }

        if (flag_num > 0){
            for (j = 0; j <  flag_num; j = j + 1){
                if (strcmp(flag, flag_list[j]) == 0){
                    if (notename_list[j] != NULL){
                        fprintf(stderr, "%s: Key '%s' found twice\n", PACKAGE_NAME, flag);
                        ret = LIST_FORMAT_ERROR;
                        goto cleanup;
                    }
                    notename_list[j] = strdup(notename);
                    if (notename_list[j] == NULL){
                        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                }
            }
        } else{
            i = i + 1;
            if (i > 1){
                putchar('\n');
            }
            result = show_one_file(flag, notename);
            if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, notename, strerror(errno));
                ret = IO_ERROR;
                goto cleanup;
            }
        }
        XFREE(flag);
        XFREE(datetime);
        XFREE(notename);
    }

    if (flag_num > 0){
        for (j = 0; j <  flag_num; j = j + 1){
            if (notename_list[j] == NULL){
                fprintf(stderr, "%s: No such note: '%s'\n", PACKAGE_NAME, flag_list[j]);
                ret = KEY_NOT_FOUND;
                // goto cleanup;
            }
        }
        first_echo = true;
        for (j = 0; j < flag_num; j = j + 1){
            if (notename_list[j] != NULL){
                if (first_echo == false){
                    putchar('\n');
                }
                result = show_one_file(flag_list[j], notename_list[j]);
                if (result != 0){
                    // fclose(fp);
                    if (result == IO_ERROR){
                        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, notename_list[j], strerror(errno));
                        ret = IO_ERROR;
                        goto cleanup;
                    }
                    fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                    ret = UNKNOWN_ERROR;
                    goto cleanup;
                }
                first_echo = false;
            }
        }
    }
    // ret will be KEY_NOT_FOUND if one or more specified keys do not exist. otherwise, ret will be 0
    goto cleanup;


cleanup:
    xfclose(&fp);

    if (notename_list != NULL){
        for (j = 0; j < flag_num; j = j + 1){
            free(notename_list[j]);
        }
    }

    free(flag);
    free(datetime);
    free(notename);
    free(notename_list);

    return ret;
}



