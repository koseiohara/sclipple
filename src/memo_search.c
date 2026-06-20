

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return IO_ERROR if failed to open file
// return REGEX_ERROR if compile failed
// return 0 otherwise
int search_one_file(char* flag, char* file, char* word){
    char*  line = NULL;
    char*  lp;
    char   errbuf[256];
    int    errcode;
    int    start;
    int    end;
    int    atty;
    int    say_linenumber;
    int    say_name;
    int    matched;
    int    ln;
    FILE*  fp;
    size_t size = 0;
    regex_t    regex;
    regmatch_t match[1];

    fp = fopen(file, "r");
    if (fp == NULL){
        perror(file);
        return IO_ERROR;
    }

    errcode = regcomp(&regex, word, REG_EXTENDED | REG_ICASE);
    if (errcode != 0){
        regerror(errcode, &regex, errbuf, sizeof(errbuf));
        // regfree(&regex);
        fclose(fp);
        return REGEX_ERROR;
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
        while (regexec(&regex, lp, 1, match, 0) == 0){
            matched = true;

            if (1 - say_name){
                if (atty){
                    printf("[\033[34m%s\033[0m]\n", flag);
                } else{
                    printf("[%s]\n", flag);
                }
                say_name = true;
            }

            if (!say_linenumber){
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
        if (matched){
            // right side of match
            printf("%s\n", lp);
        }
    }

    if (say_name){
        printf("\n");
    }

    regfree(&regex);
    fclose(fp);
    free(line);
    return 0;
}


// return IO_ERROR if failed to open list file or note
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return REGEX_ERROR if compile failed
// return KEY_NOT_FOUND if one of key does not exist
// return UNKNOWN_ERROR if program has a bug
// return 0 otherwise
int search(char* list, char* word, int flag_num, char** flag_list){
    struct stat st;
    FILE*  fp;
    char*  flag = NULL;
    char*  datetime = NULL;
    char*  notename = NULL;
    char** notename_list = NULL;
    int    result;
    int    i;
    int    j;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PROGRAM);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PROGRAM);
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
            notename_list[j] = malloc(sizeof(char));
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
                fprintf(stderr, "%s: List file is broken\n", PROGRAM);
                return LIST_FORMAT_ERROR;
            }
            fprintf(stderr, "%s: Unknown error\n", PROGRAM);
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
            result = search_one_file(flag, notename, word);
            if (result != 0){
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                free(flag);
                free(datetime);
                free(notename);
                if (result == IO_ERROR){
                    fprintf(stderr, "%s: Failed to open %s\n", PROGRAM, notename);
                    return IO_ERROR;
                } else if (result == REGEX_ERROR){
                    fprintf(stderr, "%s: regcomp failed\n", PROGRAM);
                    return REGEX_ERROR;
                }
                fprintf(stderr, "%s: Unknown error\n", PROGRAM);
                return UNKNOWN_ERROR;
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
                fprintf(stderr, "%s: No such note: '%s'\n", PROGRAM, flag_list[j]);
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                return KEY_NOT_FOUND;
            }
        }
        for (j = 0; j <  flag_num; j = j + 1){
            result = search_one_file(flag_list[j], notename_list[j], word);
            if (result != 0){
                fclose(fp);
                for (j = 0; j < flag_num; j = j + 1){
                    free(notename_list[j]);
                }
                free(notename_list);
                if (result == IO_ERROR){
                    fprintf(stderr, "%s: Failed to open %s\n", PROGRAM, notename);
                    return IO_ERROR;
                } else if (result == REGEX_ERROR){
                    fprintf(stderr, "%s: regcomp failed\n", PROGRAM);
                    return REGEX_ERROR;
                }
                fprintf(stderr, "%s: Unknown error\n", PROGRAM);
                return UNKNOWN_ERROR;
            }
        }
    }
    fclose(fp);
    for (j = 0; j < flag_num; j = j + 1){
        free(notename_list[j]);
    }
    free(notename_list);
    return 0;
}


