

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "names.h"
#include "edit_list.h"


// return IO_ERROR if failed to open file
// return 0 otherwise
int search_one_file(regex_t* regex, char* flag, char* file, int first_echo){
    char*  line = NULL;
    char*  lp;
    int    start;
    int    end;
    int    atty;
    int    say_linenumber;
    int    say_name;
    int    matched;
    int    ln;
    int    ret;
    FILE*  fp = NULL;
    size_t size = 0;
    regmatch_t match[1];

    fp = fopen(file, "r");
    if (fp == NULL){
        perror(file);
        ret = IO_ERROR;
        goto cleanup;
    }

    atty = isatty(fileno(stdout));

    rewind(fp);
    say_name = false;
    ln   = 0;
    size = 0;
    line = NULL;
    while (getline(&line, &size, fp) != -1){
        matched = false;
        say_linenumber = false;
        ln = ln + 1;

        // replace '\n' with '\0'
        line[strcspn(line, "\n")] = '\0';

        lp = line;
        while (regexec(regex, lp, 1, match, 0) == 0){
            matched = true;

            if (say_name == false){
                if (first_echo != true){
                    putchar('\n');
                }
                if (atty){
                    printf("[\033[34m%s\033[0m]\n", flag);
                } else{
                    printf("[%s]\n", flag);
                }
                say_name = true;
            }

            if (say_linenumber == false){
                if (atty){
                    printf("\033[33m%d\033[0m:", ln);
                } else{
                    printf("%d:", ln);
                }
                say_linenumber = true;
            }

            start = match[0].rm_so;
            end   = match[0].rm_eo;

            // left side of match
            printf("%.*s", start, lp);

            // match words
            if (atty){
                // change color to right
                printf("\033[31m%.*s\033[0m", end-start, lp+start);
            } else{
                // default color
                printf("%.*s", end-start, lp+start);
            }

            if (end == 0){
                if (*lp == '\0'){
                    break;
                }
                putchar(*lp);
                lp = lp + 1;
            } else{
                lp = lp + end;
            }
        }
        if (matched == true){
            // right side of match
            printf("%s\n", lp);
        }
    }

    if (ferror(fp)){
        ret = IO_ERROR;
        goto cleanup;
    }

    // if (say_name == true){
    //     printf("\n");
    // }
    if (say_name == false){
        ret = RESULT_EMPTY;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    xfclose(&fp);
    free(line);

    return ret;
}


// return IO_ERROR if failed to open list file or note
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return REGEX_ERROR if compile failed
// return KEY_NOT_FOUND if one of key does not exist
// return UNKNOWN_ERROR if program has a bug
// return 0 otherwise
int search(char* list, char* word, int flag_num, char** flag_list){
    struct  stat st;
    regex_t regex;
    FILE*  fp = NULL;
    char*  flag = NULL;
    char*  datetime = NULL;
    char*  notename = NULL;
    char** notename_list = NULL;
    char   errbuf[256];
    int    errcode;
    int    result;
    int    ret = 0;
    int    first_echo = true;
    int    i;
    int    j;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        }
        ret = IO_ERROR;
        goto cleanup;
    } 

    if (flag_num > 0){
        notename_list = malloc((size_t)flag_num * sizeof(char*));
        if (notename_list == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }

        for (j = 0; j < flag_num; j = j + 1){
            notename_list[j] = NULL;
        }
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    errcode = regcomp(&regex, word, REG_EXTENDED | REG_ICASE);
    if (errcode != 0){
        regerror(errcode, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, errbuf);
        ret = REGEX_ERROR;
        goto cleanup;
    }

    i = 0;
    while (1){
        i = i + 1;
        result = get_content_line(fp, &flag, &datetime, &notename);
        if (result == END_OF_FILE){
            break;
        } else if (result == LIST_WHITE_SPACE){
            continue;
        } else if (result != 0){
            regfree(&regex);
            if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
                goto cleanup;
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
                        regfree(&regex);
                        ret = LIST_FORMAT_ERROR;
                        goto cleanup;
                    }
                    notename_list[j] = strdup(notename);
                    if (notename_list[j] == NULL){
                        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                        regfree(&regex);
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                }
            }
        } else{
            result = search_one_file(&regex, flag, notename, first_echo);
            if (result == 0){
                first_echo = false;
            } else if (result != RESULT_EMPTY){
                regfree(&regex);
                if (result == IO_ERROR){
                    fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, notename, strerror(errno));
                    ret = IO_ERROR;
                    goto cleanup;
                }
                fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                ret = UNKNOWN_ERROR;
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
                // regfree(&regex);
                ret = KEY_NOT_FOUND;
                // goto cleanup;
            }
        }
        for (j = 0; j <  flag_num; j = j + 1){
            if (notename_list[j] != NULL){
                result = search_one_file(&regex, flag_list[j], notename_list[j], first_echo);
                if (result == 0){
                    first_echo = false;
                } else if (result != RESULT_EMPTY){
                    regfree(&regex);
                    if (result == IO_ERROR){
                        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, notename_list[j], strerror(errno));
                        ret = IO_ERROR;
                        goto cleanup;
                    }
                    fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                    ret = UNKNOWN_ERROR;
                    goto cleanup;
                }
            }
        }
    }
    // ret will be KEY_NOT_FOUND if one or more specified keys do not exist. otherwise, ret will be 0
    regfree(&regex);
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


