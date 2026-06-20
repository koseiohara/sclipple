
#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
// #include <strings.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "globals.h"
#include "strutils.h"
#include "names.h"

#define DELIM ","


// if failed to open list file or failed to write info to list, return -1
// otherwise, return 0
int write_new_content_to_list(const char* list, const char* flag, const char* datetime, const char* file){
    FILE* fp;

    fp = fopen(list, "a");
    if (fp == NULL){
        perror(list);
        return IO_ERROR;
    }

    if (fprintf(fp, "%s%s%s%s%s\n", flag, DELIM, datetime, DELIM, file) < 0){
        perror(list);
        fclose(fp);
        return IO_ERROR;
    }

    fclose(fp);
    return 0;
}


// return LIST_FORMAT_ERROR if a line does not include comma
// return INPUT_ERROR if col is too large
// return KEY_NOT_FOUND if target_flag does not exist
// return MALLOC_ERROR if strdup failed
// return 0 otherwise
int read_list_by_key(FILE* fp, char* target_flag, const int col, char** result){
    char*  line = NULL;
    char*  flag = NULL;
    int    i;
    size_t size = 0;

    while(getline(&line, &size, fp) != -1){
        line[strcspn(line, "\n")] = '\0';

        if (is_white_space(line) == true){
            continue;
        }

        flag = strtok(line, DELIM);
        if (flag == NULL){
            fprintf(stderr, "%s Error: Invalid list file. list file is broken\n", PROGRAM);
            free(line);
            return LIST_FORMAT_ERROR;
        }

        if (strcmp(flag, target_flag) != 0){
            continue;
        }

        if (col == 0){
            free(line);
            return 0;
        }
        for (i = 1; i <= col; i = i + 1){
            flag = strtok(NULL, DELIM);
            if (flag == NULL){
                fprintf(stderr, "%s Error: Invalid col. col exceeds the number of actual columns\n", PROGRAM);
                free(line);
                return INPUT_ERROR;
            }
        }
        // snprintf(result, result_len, "%s", flag);
        *result = strdup(flag);
        if (*result == NULL){
            perror("strdup");
            return MALLOC_ERROR;
        }
        free(line);
        return 0;
    }
    free(line);
    return KEY_NOT_FOUND;
}


// return false if flag does not exist
// return true if flag exist
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR otherwise
int flag_exist_check(const char* list, char* flag){
    char* dummy = NULL;
    int   stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return IO_ERROR;
    }

    stat = read_list_by_key(fp, flag, 0, &dummy);
    free(dummy);

    fclose(fp);

    if (stat == 0){
        return true;
    } else if (stat == KEY_NOT_FOUND){
        return false;
    }else if (stat == LIST_FORMAT_ERROR || stat == INPUT_ERROR){
        return LIST_FORMAT_ERROR;
    }
    return UNKNOWN_ERROR;
}


// return 0 if successed
// return IO_ERROR if failed to open list file
// return error code provided by read_list_by_key, otherwise
int get_datetime_by_key(const char* list, char* flag, char** datetime){
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return IO_ERROR;
    }

    stat = read_list_by_key(fp, flag, 1, datetime);

    fclose(fp);

    if (stat == 0){
        return 0;
    } else if (stat < 0 || stat == KEY_NOT_FOUND){
        return stat;
    }
    return UNKNOWN_ERROR;
}


// return 0 if successed
// return IO_ERROR if failed to open list file
// return error code provided by read_list_by_key, otherwise
int get_filename_by_key(const char* list, char* flag, char** filename){
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return IO_ERROR;
    }

    stat = read_list_by_key(fp, flag, 2, filename);

    fclose(fp);

    if (stat == 0){
        return 0;
    } else if (stat < 0 || stat == KEY_NOT_FOUND){
        return stat;
    }
    return UNKNOWN_ERROR;
}

// return IO_ERROR if IO failed
// return MALLOC_ERROR if asprintf failed
// return KEY_DUPLICATE if key already exist
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if old_flag does not exist
// return FILE_FORMAT_ERROR if file name is not include "--"
// return RENAME_ERROR if failed to rename tmpfile to list file
// return 0, otherwise
int mv_key_in_list(const char* list, const char* old_flag, char* new_flag){
    FILE* fpr;
    FILE* fpw;
    char* line = NULL;
    char* tmpfile  = NULL;
    char* flag     = NULL;
    char* datetime = NULL;
    char* notename = NULL;
    // char* new_notename;
    char* out_flag     = NULL;
    char* out_datetime = NULL;
    char* out_notename = NULL;
    char* dummy = NULL;
    // const char* out_flag;
    int    fd;
    int    changed;
    int    result;
    size_t size = 0;
    struct stat st;

    changed = false;

    if (stat(list, &st) != 0){
        perror(list);
        return IO_ERROR;
    }


    result = asprintf(&tmpfile, "%s.XXXXXX", list);
    if (result < 0){
        perror("asprintf");
        return MALLOC_ERROR;
    }

    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (fchmod(fd, st.st_mode) != 0){
        perror(tmpfile);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    // check existence of the new flag
    result = read_list_by_key(fpr, new_flag, 0, &dummy);
    if (result != KEY_NOT_FOUND){
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        if (result == INPUT_ERROR){
            return LIST_FORMAT_ERROR;
        } else if (result < 0){
            return result;
        }
        return KEY_DUPLICATE;
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
            fprintf(stderr, "%s: Invalid line format.\nFLAG = %s\nDATETIME = %s\nNOTENAME = %s\n", PROGRAM, flag, datetime, notename);
            fclose(fpr);
            fclose(fpw);
            free(line);
            unlink(tmpfile);
            free(tmpfile);
            return LIST_FORMAT_ERROR;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, old_flag) == 0){
            result = mv_filename(notename, new_flag, &out_notename);
            if (result < 0){
                fclose(fpr);
                fclose(fpw);
                unlink(tmpfile);
                free(line);
                // free(new_notename);
                free(tmpfile);
                return result;
            }
            // snprintf(out_flag    , sizeof(out_flag)    , "%s", new_flag);
            // snprintf(out_notename, sizeof(out_notename), "%s", new_notename);
            out_flag = strdup(new_flag);
            // out_notename = strdup(new_notename);
            changed = true;
        } else{
            // snprintf(out_flag    , sizeof(out_flag)    , "%s", flag);
            // snprintf(out_notename, sizeof(out_notename), "%s", notename);
            out_flag     = strdup(flag);
            out_notename = strdup(notename);
        }
        // snprintf(out_datetime, sizeof(out_datetime), "%s", datetime);
        out_datetime = strdup(datetime);

        // snprintf(line, sizeof(line), "%s,%s,%s\n", out_flag, out_datetime, out_notename);
        // if (fputs(line, fpw) == EOF){
        if (fprintf(fpw, "%s%s%s%s%s\n", out_flag, DELIM, out_datetime, DELIM, out_notename) < 0){
            perror(tmpfile);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            free(line);
            free(tmpfile);
            // free(new_notename);
            free(out_flag);
            free(out_notename);
            free(out_datetime);
            return IO_ERROR;
        }
        free(out_flag);
        free(out_datetime);
        free(out_notename);

        out_flag     = NULL;
        out_datetime = NULL;
        out_notename = NULL;
    }

    free(line);
    // free(new_notename);
    free(out_flag);
    free(out_notename);
    free(out_datetime);

    if (ferror(fpr)){
        perror(list);
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (fclose(fpr)){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (fclose(fpw)){
        perror(tmpfile);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (rename(tmpfile, list) != 0){
        perror("list rename");
        unlink(tmpfile);
        free(tmpfile);
        return RENAME_ERROR;
    }
    
    free(tmpfile);

    if (changed == false){
        return KEY_NOT_FOUND;
    }

    return 0;
}


// return IO_ERROR if IO failed
// return MALLOC_ERROR if MALLOC failed
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if target_flag does not exist
// return 0 otherwise
int rm_key_in_list(const char* list, const char* target_flag){
    FILE* fpr;
    FILE* fpw;
    char* line     = NULL;
    char* tmpfile  = NULL;
    char* flag     = NULL;
    char* datetime = NULL;
    char* notename = NULL;
    int   fd;
    int   removed;
    int   result;
    struct stat st;
    size_t size = 0;

    removed = false;

    if (stat(list, &st) != 0){
        perror(list);
        return IO_ERROR;
    }

    result = asprintf(&tmpfile, "%s.XXXXXX", list);
    if (result < 0){
        perror("asprintf");
        return MALLOC_ERROR;
    }

    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (fchmod(fd, st.st_mode) != 0){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
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
            fprintf(stderr, "%s: Invalid line format.\nFLAG = %s\nDATETIME = %s\nNOTENAME = %s\n", PROGRAM, flag, datetime, notename);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            free(tmpfile);
            free(line);
            return LIST_FORMAT_ERROR;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            removed = true;
            continue;
        }

        // snprintf(out_line, sizeof(out_line), "%s,%s,%s\n", flag, datetime, notename);
        if (fprintf(fpw, "%s%s%s%s%s\n", flag, DELIM, datetime, DELIM, notename) < 0){
            perror(tmpfile);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            free(tmpfile);
            free(line);
            return IO_ERROR;
        }
    }

    free(line);

    if (ferror(fpr)){
        perror(list);
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (fclose(fpr)){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (fclose(fpw)){
        perror(tmpfile);
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    if (rename(tmpfile, list) != 0){
        perror("list rename");
        unlink(tmpfile);
        free(tmpfile);
        return IO_ERROR;
    }

    free(tmpfile);

    if (removed == false){
        return KEY_NOT_FOUND;
    }

    return 0;
}


// return END_OF_FILE if getline failed
// return LIST_WHITE_SPACE if line is empty
// return LIST_FORMAT_ERROR if list file is broken
// return 0 otherwise
int get_content_line(FILE* fp, char** flag, char** datetime, char** notename){
    char*  line = NULL;
    char*  in_flag     = NULL;
    char*  in_datetime = NULL;
    char*  in_notename = NULL;
    char*  dummy = NULL;
    size_t size  = 0;

    if (getline(&line, &size, fp) == -1){
        free(line);
        return END_OF_FILE;
    }

    if (is_white_space(line) == true){
        free(line);
        return LIST_WHITE_SPACE;
    }

    line[strcspn(line, "\n")] = '\0';

    in_flag     = strtok(line, DELIM);
    in_datetime = strtok(NULL, DELIM);
    in_notename = strtok(NULL, DELIM);
    dummy       = strtok(NULL, DELIM);

    if (in_flag == NULL || in_datetime == NULL || in_notename == NULL || dummy != NULL){
        free(line);
        return LIST_FORMAT_ERROR;
    }

    *flag     = strdup(in_flag);
    *datetime = strdup(in_datetime);
    *notename = strdup(in_notename);

    free(line);

    return 0;
}


