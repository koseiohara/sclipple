

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


// return -1 when io error or regex error
// return 0 otherwise
int search_one_file(char* flag, char* file, char* word){
    char*  line;
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
    size_t size;
    regex_t    regex;
    regmatch_t match[1];

    fp = fopen(file, "r");
    if (fp == NULL){
        perror(file);
        return -1;
    }


    errcode = regcomp(&regex, word, REG_EXTENDED | REG_ICASE);
    if (errcode != 0){
        regerror(errcode, &regex, errbuf, sizeof(errbuf));
        regfree(&regex);
        fprintf(stderr, "%s Error: regcomp failed\n", PROGRAM);
        fclose(fp);
        return -1;
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

            if (1 - say_linenumber){
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


int search(char* list, char* word, int flag_num, char** flag_list){
    struct stat st;
    FILE* fp;
    char  flag[FLAG_LEN];
    char  datetime[DATETIME_LEN];
    char  notename[FILE_APATH_LEN];
    char  (*notename_list)[FILE_APATH_LEN] = NULL;
    int   result;
    int   i;
    int   j;

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
            if (search_one_file(flag, notename, word) != 0){
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
            if (search_one_file(flag_list[j], notename_list[j], word) != 0){
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


