
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "strutils.h"
#include "names.h"

#define DELIM ","


// return IO_ERROR if failed to open or write to list file
// return 0 otherwise
int write_new_content_to_list(const char* list, const char* flag, const char* datetime, const char* file){
    FILE* fp = NULL;
    int ret = 0;

    fp = fopen(list, "a");
    if (fp == NULL){
        ret = IO_ERROR;

        goto cleanup;
    }

    if (fprintf(fp, "%s%s%s%s%s\n", flag, DELIM, datetime, DELIM, file) < 0){
        ret = IO_ERROR;

        goto cleanup;
    }

cleanup:
    if (xfclose(&fp) != 0){
        if (ret == 0){
            ret = IO_ERROR;
        }
    }
    return ret;
}


// return LIST_FORMAT_ERROR if a line does not include comma
// return INPUT_ERROR if col is too large
// return KEY_NOT_FOUND if target_flag does not exist
// return IO_ERROR if getline failed
// return MALLOC_ERROR if strdup failed
// return 0 otherwise
int read_list_by_key(FILE* fp, char* target_flag, const int col, char** result){
    char*  line = NULL;
    char*  flag;
    int    i;
    int    ret;
    size_t size = 0;

    while(getline(&line, &size, fp) != -1){
        line[strcspn(line, "\n")] = '\0';

        if (is_white_space(line) == true){
            continue;
        }

        flag = strtok(line, DELIM);
        if (flag == NULL){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        }

        if (strcmp(flag, target_flag) != 0){
            continue;
        }

        if (col == 0){
            ret = 0;
            goto cleanup;
        }
        for (i = 1; i <= col; i = i + 1){
            flag = strtok(NULL, DELIM);
            if (flag == NULL){
                fprintf(stderr, "%s: Invalid col. col exceeds the number of actual columns\n", PACKAGE_NAME);
                ret = INPUT_ERROR;
                goto cleanup;
            }
        }
        *result = strdup(flag);
        if (*result == NULL){
            ret = MALLOC_ERROR;
            goto cleanup;
        }
        ret = 0;
        goto cleanup;
    }

    if (ferror(fp)){
        ret = IO_ERROR;
        goto cleanup;
    }

    ret = KEY_NOT_FOUND;
    goto cleanup;

cleanup:
    free(line);
    return ret;
}


// return false if flag does not exist
// return true if flag exist
// return IO_ERROR if failed to open list
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR otherwise
int flag_exist_check(const char* list, char* flag){
    char* dummy = NULL;
    int   stat;
    int   ret;
    FILE* fp = NULL;

    fp = fopen(list, "r");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    stat = read_list_by_key(fp, flag, 0, &dummy);
    XFREE(dummy);

    xfclose(&fp);

    if (stat == 0){
        ret = true;
        goto cleanup;
    } else if (stat == KEY_NOT_FOUND){
        ret = false;
        goto cleanup;
    } else if (stat == LIST_FORMAT_ERROR || stat == INPUT_ERROR){
        ret = LIST_FORMAT_ERROR;
        goto cleanup;
    } else if (stat == IO_ERROR){
        ret = IO_ERROR;
        goto cleanup;
    }
    fprintf(stderr, "%s: Unknown Error in flag_exist_check()\n", PACKAGE_NAME);
    ret = UNKNOWN_ERROR;
    goto cleanup;

cleanup:
    free(dummy);
    xfclose(&fp);
    return ret;
}


// return 0 if successed
// return IO_ERROR if failed to open list file
// return error code provided by read_list_by_key, otherwise
int get_datetime_by_key(const char* list, char* flag, char** datetime){
    int  stat;
    int  ret;
    FILE* fp = NULL;

    fp = fopen(list, "r");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    stat = read_list_by_key(fp, flag, 1, datetime);

    xfclose(&fp);

    if (stat == 0){
        ret = 0;
        goto cleanup;
    } else if (stat == KEY_NOT_FOUND || stat == LIST_FORMAT_ERROR || stat == INPUT_ERROR || stat == IO_ERROR || stat == MALLOC_ERROR){
        ret = stat;
        goto cleanup;
    }
    fprintf(stderr, "%s: Unknown Error in flag_exist_check()\n", PACKAGE_NAME);
    ret = UNKNOWN_ERROR;
    goto cleanup;

cleanup:
    xfclose(&fp);
    return ret;
}


// return 0 if successed
// return IO_ERROR if failed to open list file
// return error code provided by read_list_by_key, otherwise
int get_filename_by_key(const char* list, char* flag, char** filename){
    int  stat;
    int  ret;
    FILE* fp = NULL;

    fp = fopen(list, "r");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    stat = read_list_by_key(fp, flag, 2, filename);

    xfclose(&fp);

    if (stat == 0){
        ret = 0;
        goto cleanup;
    } else if (stat == KEY_NOT_FOUND || stat == LIST_FORMAT_ERROR || stat == INPUT_ERROR || stat == IO_ERROR || stat == MALLOC_ERROR){
        ret = stat;
        goto cleanup;
    }
    ret = UNKNOWN_ERROR;
    goto cleanup;

cleanup:
    xfclose(&fp);
    return ret;
}

// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc failed
// return KEY_DUPLICATE if key already exist
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if old_flag does not exist
// return FILE_FORMAT_ERROR if file name is not include "--"
// return RENAME_ERROR if failed to rename tmpfile to list file
// return 0, otherwise
int mv_key_in_list(const char* list, const char* old_flag, char* new_flag, char* new_file){
    FILE* fpr  = NULL;
    FILE* fpw  = NULL;
    char* line = NULL;
    char* tmpfile  = NULL;
    char* out_flag     = NULL;
    char* out_datetime = NULL;
    char* out_notename = NULL;
    char* dummy = NULL;
    char* flag;
    char* datetime;
    char* notename;
    int    fd;
    int    changed;
    int    result;
    int    ret;
    size_t size = 0;
    struct stat st;

    changed = false;

    if (stat(list, &st) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }


    result = asprintf(&tmpfile, "%s.XXXXXX", list);
    if (result < 0){
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    fd = mkstemp(tmpfile);
    if (fd == -1){
        ret = IO_ERROR;
        goto cleanup;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        unlink(tmpfile);
        close(fd);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (fchmod(fd, st.st_mode) != 0){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    // check existence of the new flag
    result = read_list_by_key(fpr, new_flag, 0, &dummy);
    XFREE(dummy);
    if (result != KEY_NOT_FOUND){
        unlink(tmpfile);
        if (result == INPUT_ERROR){
            ret = LIST_FORMAT_ERROR;
        } else if (result == 0){
            ret = KEY_DUPLICATE;
        } else{
            ret = result;
        }
        goto cleanup;
    }
    #ifdef DEBUG
    printf("Completed checking flag list\n");
    #endif
    rewind(fpr);
    #ifdef DEBUG
    printf("rewind successed\n");
    #endif

    // rename loop
    while (getline(&line, &size, fpr) != -1){
        // replace '\n' to '\0'
        line[strcspn(line, "\n")] = '\0';

        // skip empty line
        if (is_white_space(line) == true){
            continue;
        }

        // find the first delimiter
        flag     = strtok(line, DELIM);
        datetime = strtok(NULL, DELIM);
        notename = strtok(NULL, DELIM);
        #ifdef DEBUG
        printf("<DEBUG> FLAG    : %s\n", flag);
        printf("<DEBUG> DATETIME: %s\n", datetime);
        printf("<DEBUG> NOTENAME: %s\n", notename);
        #endif

        if (flag == NULL || datetime == NULL || notename == NULL){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            unlink(tmpfile);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, old_flag) == 0){
            out_flag     = strdup(new_flag);
            out_notename = strdup(new_file);
            if (out_flag == NULL || out_notename == NULL){
                unlink(tmpfile);
                ret = MALLOC_ERROR;
                goto cleanup;
            }
            changed = true;
        } else{
            out_flag     = strdup(flag);
            out_notename = strdup(notename);
            if (out_flag == NULL || out_notename == NULL){
                unlink(tmpfile);
                ret = MALLOC_ERROR;
                goto cleanup;
            }
        }
        out_datetime = strdup(datetime);
        if (out_datetime == NULL){
            unlink(tmpfile);
            ret = MALLOC_ERROR;
            goto cleanup;
        }

        if (fprintf(fpw, "%s%s%s%s%s\n", out_flag, DELIM, out_datetime, DELIM, out_notename) < 0){
            unlink(tmpfile);
            ret = IO_ERROR;
            goto cleanup;
        }
        XFREE(out_flag);
        XFREE(out_datetime);
        XFREE(out_notename);
    }

    XFREE(line);
    XFREE(out_flag);
    XFREE(out_notename);
    XFREE(out_datetime);

    if (ferror(fpr)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (xfclose(&fpr)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (xfclose(&fpw)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (rename(tmpfile, list) != 0){
        unlink(tmpfile);
        ret = RENAME_ERROR;
        goto cleanup;
    }
    
    XFREE(tmpfile);

    if (changed == false){
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    xfclose(&fpr);
    xfclose(&fpw);

    free(line);
    free(tmpfile);
    free(out_flag);
    free(out_datetime);
    free(out_notename);
    free(dummy);

    return ret;
}


// return IO_ERROR if IO failed
// return MALLOC_ERROR if MALLOC failed
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if target_flag does not exist
// return 0 otherwise
int rm_key_in_list(const char* list, const char* target_flag){
    FILE* fpr     = NULL;
    FILE* fpw     = NULL;
    char* line    = NULL;
    char* tmpfile = NULL;
    char* flag;
    char* datetime;
    char* notename;
    int   fd;
    int   removed;
    int   result;
    int   ret;
    struct stat st;
    size_t size = 0;

    removed = false;

    if (stat(list, &st) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    result = asprintf(&tmpfile, "%s.XXXXXX", list);
    if (result < 0){
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    fd = mkstemp(tmpfile);
    if (fd == -1){
        ret = IO_ERROR;
        goto cleanup;
    }

    if (fchmod(fd, st.st_mode) != 0){
        close(fd);
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        close(fd);
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    while (getline(&line, &size, fpr) != -1){
        // replace '\n' to '\0'
        line[strcspn(line, "\n")] = '\0';

        // skip empty line
        if (is_white_space(line) == true){
            continue;
        }

        // find the first delimiter
        flag     = strtok(line, DELIM);
        datetime = strtok(NULL, DELIM);
        notename = strtok(NULL, DELIM);
        #ifdef DEBUG
        printf("<DEBUG> FLAG : %s\n", flag);
        printf("<DEBUG> DATETIME: %s\n", datetime);
        printf("<DEBUG> NOTENAME: %s\n", notename);
        #endif

        if (flag == NULL || datetime == NULL || notename == NULL){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            unlink(tmpfile);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            removed = true;
            continue;
        }

        // snprintf(out_line, sizeof(out_line), "%s,%s,%s\n", flag, datetime, notename);
        if (fprintf(fpw, "%s%s%s%s%s\n", flag, DELIM, datetime, DELIM, notename) < 0){
            unlink(tmpfile);
            ret = IO_ERROR;
            goto cleanup;
        }
    }

    XFREE(line);

    if (ferror(fpr)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (xfclose(&fpr)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (xfclose(&fpw)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (rename(tmpfile, list) != 0){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    XFREE(tmpfile);

    if (removed == false){
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    xfclose(&fpr);
    xfclose(&fpw);
    free(line);
    free(tmpfile);

    return ret;
}


// return END_OF_FILE if getline failed
// return LIST_WHITE_SPACE if line is empty
// return LIST_FORMAT_ERROR if list file is broken
// return MALLOC_ERROR if malloc failed
// return IO_ERROR if IO failed
// return 0 otherwise
int get_content_line(FILE* fp, char** flag, char** datetime, char** notename){
    char*  line = NULL;
    char*  dummy = NULL;
    char*  in_flag;
    char*  in_datetime;
    char*  in_notename;
    int    ret;
    size_t size  = 0;

    *flag     = NULL;
    *datetime = NULL;
    *notename = NULL;

    if (getline(&line, &size, fp) == -1){
        if (ferror(fp)){
            ret = IO_ERROR;
            goto cleanup;
        }
        ret = END_OF_FILE;
        goto cleanup;
    }

    if (is_white_space(line) == true){
        ret = LIST_WHITE_SPACE;
        goto cleanup;
    }

    line[strcspn(line, "\n")] = '\0';

    in_flag     = strtok(line, DELIM);
    in_datetime = strtok(NULL, DELIM);
    in_notename = strtok(NULL, DELIM);
    dummy       = strtok(NULL, DELIM);

    if (in_flag == NULL || in_datetime == NULL || in_notename == NULL || dummy != NULL){
        ret = LIST_FORMAT_ERROR;
        goto cleanup;
    }

    *flag     = strdup(in_flag);
    *datetime = strdup(in_datetime);
    *notename = strdup(in_notename);
    if (*flag == NULL || *datetime == NULL || *notename == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    free(line);

    return ret;
}


