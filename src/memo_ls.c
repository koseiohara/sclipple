

#include "config.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "file_systems.h"
#include "strutils.h"
#include "list_utils.h"
#include "memo_ls.h"

// #include "edit_list.h"


// #define OUTPUT_TTY  "[\033[34m%s\033[0m]\ncreated: %s\nfile   : %s\n%s\n\n"
#define OUTPUT_TTY  "[\033[34m%s\033[0m]\n\033[36mcreated\033[0m: %s\n\033[36mfile\033[0m   : %s\n\033[36mtags\033[0m   : %s\n%s\n"
#define OUTPUT_NTTY "[%s]\ncreated: %s\nfile   : %s\ntags   :%s\n%s\n"

// return IO_ERROR if failed to open note
// return RESULT_EMPTY if note is empty
// return 0 otherwise
static inline int get_first_line(const char* notename, const int first_line_len, char* first_line){
    FILE* fp = NULL;
    char* enter;
    int ret;

    fp = fopen(notename, "r");
    if (fp == NULL){
        return IO_ERROR;
    }

    while (fgets(first_line, first_line_len-3, fp) != NULL){
        if (is_white_space(first_line) == true){
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

        ret = 0;
        goto cleanup;
    }

    if (ferror(fp)){
        ret = IO_ERROR;
        goto cleanup;
    }

    first_line[0] = '\0';
    ret = RESULT_EMPTY;
    goto cleanup;

cleanup:
    if (xfclose(&fp)){
        if (ret == 0 || ret == RESULT_EMPTY){
            ret = IO_ERROR;
        }
    }

    return ret;
}


static inline int ls_with_key_tag(const int tty, const char* list, const int nkeys, char* const* keys, const int ntags, char* const* tags){
    FILE*      fp           = NULL;
    ListField* field_by_key = NULL;
    ListField* field_by_tag = NULL;
    ListField* field_merged = NULL;
    ListField* work_list;
    char** unfound  = NULL;
    char** tags_all = NULL;
    char*  line     = NULL;
    char*  tagline  = NULL;
    char** work_tags;
    char*  date;
    char   first_line[LS_LINE_LEN];
    int    ntags_all;
    int    found_by_keys;
    int    found_by_tags;
    int    nconts;
    int    first_echo = true;
    int    result;
    int    ret = 0;
    int    i;
    int    j;

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

    // lines = malloc((size_t)nconts * sizeof(char*));
    // if (lines == NULL){
    //     fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
    //     ret = MALLOC_ERROR;
    //     goto cleanup;
    // }

    for (j = 0; j < nconts; j = j + 1){
        if (first_echo == false){
            putchar('\n');
        }
        first_echo = false;

        work_list = &field_merged[j];

        result = get_first_line(work_list->file, LS_LINE_LEN, first_line);
        if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, work_list->file, strerror(errno));
            ret = IO_ERROR;
            goto cleanup;
        }

        if (work_list->meta != NULL){
            result = parse_meta(work_list->meta, &date, &ntags_all, &tags_all);
            if (result != 0){
                if (result == LIST_FORMAT_ERROR){
                    fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                    ret = LIST_FORMAT_ERROR;
                } else if (result == MALLOC_ERROR){
                    fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                    ret = MALLOC_ERROR;
                }
                goto cleanup;
            }
            work_tags = tags_all;
        } else{
            date      = work_list->date;
            ntags_all = work_list->ntags;
            work_tags = work_list->tags;
        }

        result = tags2line(ntags_all, work_tags, &tagline);
        if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }

        if (tty){
            // result = asprintf(&lines[j], OUTPUT_TTY , work_list->key, date, work_list->file, tagline, first_line);
            printf(OUTPUT_TTY , work_list->key, date, work_list->file, tagline, first_line);
        } else{
            // result = asprintf(&lines[j], OUTPUT_NTTY, work_list->key, date, work_list->file, tagline, first_line);
            printf(OUTPUT_NTTY, work_list->key, date, work_list->file, tagline, first_line);
        }
        XFREE(tags_all);
        XFREE(tagline);
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
    free(line);
    free(unfound);
    free(tags_all);
    free(tagline);

    return ret;
}


static inline int ls_without_key_tag(const int tty, const char* list){
    FILE*  fp       = NULL;
    char*  line     = NULL;
    char** tags_all = NULL;
    char*  tagline  = NULL;
    char*  work_line;
    char*  key;
    char*  file;
    char*  meta;
    char*  date;
    char   first_line[LS_LINE_LEN];
    int    first_echo = true;
    int    ntags_all;
    int    result;
    int    ret = 0;
    size_t size = 0;

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    while (getline(&line, &size, fp) != -1){
        if (first_echo == false){
            putchar('\n');
        }
        first_echo = false;

        work_line = line;
        result = parse_line(&work_line, &key, &file, &meta);
        if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        }

        result = get_first_line(file, LS_LINE_LEN, first_line);
        if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, file, strerror(errno));
            ret = IO_ERROR;
            goto cleanup;
        }

        result = parse_meta(meta, &date, &ntags_all, &tags_all);
        if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
            }
            goto cleanup;
        }

        result = tags2line(ntags_all, tags_all, &tagline);
        if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }
        XFREE(tags_all);

        if (tty){
            // result = asprintf(&lines[j], OUTPUT_TTY , work_list->key, date, work_list->file, tagline, first_line);
            fprintf(stdout, OUTPUT_TTY , key, date, file, tagline, first_line);
        } else{
            // result = asprintf(&lines[j], OUTPUT_NTTY, work_list->key, date, work_list->file, tagline, first_line);
            fprintf(stdout, OUTPUT_NTTY, key, date, file, tagline, first_line);
        }
        XFREE(tagline);
    }

    if (ferror(fp)){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    goto cleanup;


cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
    }
    free(line);
    free(tags_all);
    free(tagline);

    return ret;
}


// return IO_ERROR if list file does not exist, failed to open list file, or failed to read the first line of note
// return INPUT_ERROR if nkeys is a negative value
// return MALLOC_ERROR if malloc or asprintf failed
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if one or more flags is not found
// return UNKNOWN_ERROR if error handling is not enough
// return 0 otherwise
int ls(const char* list, int nkeys, char** keys, int ntags, char** tags){
    struct stat st;
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

    tty = isatty(fileno(stdout));

    // check the existence of the list file
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

    if (nkeys < 0 || ntags < 0){
        fprintf(stderr, "%s: Invalid number of keys or tags: keys=%d, tags=%d\n", PACKAGE_NAME, nkeys, ntags);
        ret = INPUT_ERROR;
        goto cleanup;
    } else if (nkeys > 0 || ntags > 0){
        result = ls_with_key_tag(tty, list, nkeys, keys, ntags, tags);
        if (result != 0){
            ret = result;
            goto cleanup;
        }
    } else{
        result = ls_without_key_tag(tty, list);
        if (result != 0){
            ret = result;
            goto cleanup;
        }
    }

    // ret will be KEY_NOT_FOUND if one or more specified keys do not exist. otherwise, ret will be 0
    goto cleanup;

cleanup:
    return ret;
}



