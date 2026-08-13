

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <regex.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "file_systems.h"
#include "list_utils.h"
#include "memo_search.h"


// return IO_ERROR if failed to open file
// return 0 otherwise
int search_one_file(regex_t* regex, int tty, char* key, char* file, int first_echo){
    char*  line = NULL;
    char*  lp;
    int    start;
    int    end;
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
                if (tty){
                    printf("[\033[34m%s\033[0m]\n", key);
                } else{
                    printf("[%s]\n", key);
                }
                say_name = true;
            }

            if (say_linenumber == false){
                if (tty){
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
            if (tty){
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
    if (xfclose(&fp)){
        if (ret == 0 || ret == RESULT_EMPTY){
            ret = IO_ERROR;
        }
    }
    free(line);

    return ret;
}


static inline int search_with_key_tag(regex_t* regex, const int tty, const char* list, const int nkeys, char* const* keys, const int ntags, char* const* tags){
    FILE*      fp           = NULL;
    ListField* field_by_key = NULL;
    ListField* field_by_tag = NULL;
    ListField* field_merged = NULL;
    ListField* work_list;
    char** unfound = NULL;
    int    found_by_keys;
    int    found_by_tags;
    int    nconts;
    int    first_echo = true;
    int    result;
    int    ret = 0;
    int    i;

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    if (nkeys > 0){
        unfound = malloc((size_t)nkeys * sizeof(char*));
        if (unfound == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }
    } else{
        unfound = malloc(sizeof(char*));
        if (unfound == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }
        unfound[0] = NULL;
    }
    result = get_content_by_key_and_tag(fp, nkeys, keys, &found_by_keys, unfound, 
                                        ntags, tags, &found_by_tags, 
                                        &nconts, &field_merged, &field_by_key, &field_by_tag);
    if (result != 0){
        if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
        } else if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    for (i = 0; i < nkeys && unfound[i] != NULL; i = i + 1){
        fprintf(stderr, "%s: No such key: %s\n", PACKAGE_NAME, unfound[i]);
        ret = KEY_NOT_FOUND;
    }

    if (nconts == 0){
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }

    for (i = 0; i < nconts; i = i + 1){
        work_list = &field_merged[i];

        result = search_one_file(regex, tty, work_list->key, work_list->file, first_echo);
        if (result == 0){
            first_echo = false;
        } else if (result != RESULT_EMPTY){
            if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, work_list->file, strerror(errno));
                ret = IO_ERROR;
                goto cleanup;
            }
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
            goto cleanup;
        }
    }

cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
    }

    if (field_by_key != NULL){
        for (i = 0; i < nkeys; i = i + 1){
            free_ListField(&(field_by_key[i]));
        }
    }
    if (field_by_tag != NULL){
        for (i = 0; i < found_by_tags; i = i + 1){
            free_ListField(&(field_by_tag[i]));
        }
    }
    free(field_by_key);
    free(field_by_tag);
    free(field_merged);
    free(unfound);
    return ret;
}


static inline int search_without_key_tag(regex_t* regex, const int tty, const char* list){
    FILE* fp;
    char* line = NULL;
    char* work_line;
    char* key;
    char* file;
    int result;
    int ret = 0;
    int first_echo = true;
    size_t size = 0;

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    while (getline(&line, &size, fp) != -1){
        work_line = line;
        result = parse_line(&work_line, &key, &file, NULL);
        if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        }

        result = search_one_file(regex, tty, key, file, first_echo);
        if (result == 0){
            first_echo = false;
        } else if (result != RESULT_EMPTY){
            if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, file, strerror(errno));
                ret = IO_ERROR;
                goto cleanup;
            }
            fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
            goto cleanup;
        }
    }

    if (ferror(fp)){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }


cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
    }
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
int search(char* list, char* word, int nkeys, char** keys, int ntags, char** tags){
    struct  stat st;
    regex_t regex;
    char   errbuf[256];
    int    tty;
    int    result;
    int    ret = 0;

    if (nkeys < 0 || ntags < 0){
        if (nkeys < 0){
            fprintf(stderr, "%s: Unknown error: No keys were speicified to add\n", PACKAGE_NAME);
        } else{
            fprintf(stderr, "%s: Unknown error: Number of tags is negative\n", PACKAGE_NAME);
        }
        ret = INPUT_ERROR;
        goto cleanup;
    }

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

    result = regcomp(&regex, word, REG_EXTENDED | REG_ICASE);
    if (result != 0){
        regerror(result, &regex, errbuf, sizeof(errbuf));
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, errbuf);
        ret = REGEX_ERROR;
        goto cleanup;
    }

    tty = isatty(fileno(stdout));

    if (nkeys < 0 || ntags < 0){
        fprintf(stderr, "%s: Invalid number of keys or tags: keys=%d, tags=%d\n", PACKAGE_NAME, nkeys, ntags);

        regfree(&regex);
        ret = INPUT_ERROR;
        goto cleanup;
    } else if (nkeys > 0 || ntags > 0){
        result = search_with_key_tag(&regex, tty, list, nkeys, keys, ntags, tags);
    } else{
        result = search_without_key_tag(&regex, tty, list);
    }

    regfree(&regex);

    ret = result;
    goto cleanup;

cleanup:
    return ret;
}


