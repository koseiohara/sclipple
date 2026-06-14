

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>

#include "globals.h"
#include "edit_list.h"


// return -1 when io error or regex error
// return 0 otherwise
int search_one_file(char* file, char* word){
    char  line[SEARCH_LINE_LEN];
    char* lp;
    int   start;
    int   end;
    int   atty;
    int   say_name;
    int   matched;
    FILE* fp;
    regex_t    regex;
    regmatch_t match[1];

    fp = fopen(file, "r");
    if (fp == NULL){
        perror(file);
        return -1;
    }

    if (regcomp(&regex, word, REG_EXTENDED | REG_ICASE) != 0){
        fprintf(stderr, "%s: regcomp failed\n", PROGRAM);
        fclose(fp);
        return -1;
    }

    atty = isatty(fileno(stdout));

    rewind(fp);
    say_name = false;
    while (fgets(line, sizeof(line), fp) != NULL){
        matched  = false;

        // replace '\n' with '\0'
        line[strcspn(line, "\n")] = '\0';

        lp = line;
        while (regexec(&regex, lp, 1, match, 0) == 0){
            matched = true;

            if (1 - say_name){
                if (atty){
                    printf("\033[34m%s\033[0m\n", file);
                } else{
                    printf("%s\n", file);
                }
                say_name = true;
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

    regfree(&regex);
    fclose(fp);
    return 0;
}


int search(char* list, char* word, int flag_num, char** flag_list){
    FILE* fp;
    char  flag[FLAG_LEN];
    char  datetime[DATETIME_LEN];
    char  notename[FILE_APATH_LEN];
    char  (*notename_list)[FILE_APATH_LEN] = NULL;
    int   result;
    int   i;
    int   j;

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
            fprintf(stderr, "%s: Invalid line is found in list file\nlist file is broken in line %d\n", PROGRAM, i);
            free(notename_list);
            fclose(fp);
            return -1;
        }

        if (flag_num > 0){
            for (j = 0; j <  flag_num; j = j + 1){
                if (strcmp(flag, flag_list[j]) == 0){
                    strcpy(notename_list[j], notename);
                }
            }
        } else{
            if (search_one_file(notename, word) != 0){
                free(notename_list);
                fclose(fp);
                return -1;
            }
        }
    }

    if (flag_num > 0){
        for (j = 0; j <  flag_num; j = j + 1){
            if (notename_list[j][0] == '\0'){
                fprintf(stderr, "%s: Note '%s' was not found\n", PROGRAM, flag_list[j]);
                free(notename_list);
                fclose(fp);
                return -1;
            }
        }
        for (j = 0; j <  flag_num; j = j + 1){
            if (search_one_file(notename_list[j], word) != 0){
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


