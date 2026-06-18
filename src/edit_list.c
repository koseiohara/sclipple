
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
// int write_new_content_to_list(const char* list, const char* flag, const char* datetime, const char* file){
//     int fd;
//     // char list_path[DIR_LEN+FILE_LEN+1];
//     char content[FLAG_LEN+1+DATETIME_LEN+1+FILE_APATH_LEN+16];

//     fd = open(list, O_WRONLY | O_APPEND);
//     if (fd == -1){
//         perror(list);
//         return -1;
//     }
//     snprintf(content, sizeof(content), "%s,%s,%s\n", flag, datetime, file);

//     #ifdef DEBUG
//     printf("%s\n", content);
//     #endif
//     if (write(fd, content, strlen(content)) == -1){
//         perror(list);
//         close(fd);
//         return -1;
//     }

//     close(fd);
//     return 0;
// }


// if failed to open list file or failed to write info to list, return -1
// otherwise, return 0
int write_new_content_to_list(const char* list, const char* flag, const char* datetime, const char* file){
    FILE* fp;

    fp = fopen(list, "a");
    if (fp == NULL){
        perror(list);
        return IO_ERROR;
    }

    if (fprintf(fp, "%s,%s,%s\n", flag, datetime, file) < 0){
        perror(list);
        fclose(fp);
        return IO_ERROR;
    }

    fclose(fp);
    return 0;
}


// return -2 if list file is broken
// return -1 if col is too large or list element in a line is less than 3
// return  0 if target line is found
// return  1 if flag does not exist
int read_list_by_key(FILE* fp, char* target_flag, const int col, size_t result_len, char* result){
    // char   line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char*  line;
    char*  flag;
    int    i;
    size_t size;

    while(getline(&line, &size, fp) != -1){
        line[strcspn(line, "\n")] = '\0';

        if (is_white_space(line) == true){
            continue;
        }

        flag = strtok(line, DELIM);
        if (flag == NULL){
            fprintf(stderr, "%s Error: Invalid list file. list file is broken\n", PROGRAM);
            return -2;
        }

        if (strcmp(flag, target_flag) != 0){
            continue;
        }

        if (col == 0){
            return 0;
        }
        for (i = 1; i <= col; i = i + 1){
            flag = strtok(NULL, DELIM);
            if (flag == NULL){
                fprintf(stderr, "%s Error: Invalid col. col exceeds the number of actual columns\n", PROGRAM);
                return -1;
            }
        }
        snprintf(result, result_len, "%s", flag);
        return 0;
    }
    return 1;
}


// return 0 if flag does not exist
// return -1 if flag exist
int flag_exist_check(const char* list, char* flag){
    char dummy[128];
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return -2;
    }

    stat = read_list_by_key(fp, flag, 0, sizeof(dummy), dummy);

    fclose(fp);

    if (stat == 1){
        return 0;
    } else if (stat == 0){
        return -1;
    } else if (stat < 0){
        exit(1);
    }
    return -2;
}


int get_datetime_by_key(const char* list, char* flag, size_t datetime_len, char* datetime){
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return -2;
    }

    stat = read_list_by_key(fp, flag, 1, datetime_len, datetime);

    fclose(fp);

    if (stat < 0 || stat == 1){
        return -1;
    }
    return 0;
}


int get_filename_by_key(const char* list, char* flag, size_t filename_len, char* filename){
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return -2;
    }

    stat = read_list_by_key(fp, flag, 2, filename_len, filename);

    fclose(fp);

    if (stat < 0 || stat == 1){
        return -1;
    }
    return 0;
}


// IO error, return -1
// if failed to copy temporary file to list, return -2
// list broken error, return -3
// if flag is not found, return 1
// if new flag is exist, return 2
// otherwise, return 0
int mv_key_in_list(const char* list, const char* old_flag, char* new_flag){
    FILE* fpr;
    FILE* fpw;
    // char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char* line;
    // char  tmpfile[LIST_APATH_LEN+8];
    char* tmpfile;
    char* flag;
    char* datetime;
    char* notename;
    // char* new_notename;
    char* out_flag;
    char* out_datetime;
    char* out_notename;
    char  dummy[128];
    // const char* out_flag;
    int   fd;
    int   changed;
    int   result;
    int   len;
    size_t size;
    struct stat st;

    changed = 0;

    if (stat(list, &st) != 0){
        perror(list);
        return -1;
    }


    len = strlen(list);
    tmpfile = malloc((len+8) * sizeof(char));
    snprintf(tmpfile, sizeof(tmpfile), "%s.XXXXXX", list);
    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        free(tmpfile);
        return -1;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (fchmod(fd, st.st_mode) != 0){
        perror(tmpfile);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    // check existence of the new flag
    result = read_list_by_key(fpr, new_flag, 0, sizeof(dummy), dummy);
    if (result != 1){
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        if (result < 0){
            fprintf(stderr, "%s Error: Invalid list file. list file is broken\n", PROGRAM);
            exit(1);
        }
        return 2;
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
            return -1;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, old_flag) == 0){
            if (mv_filename(notename, new_flag, out_notename) < 0){
                fclose(fpr);
                fclose(fpw);
                unlink(tmpfile);
                free(line);
                // free(new_notename);
                free(tmpfile);
                return -3;
            }
            // snprintf(out_flag    , sizeof(out_flag)    , "%s", new_flag);
            // snprintf(out_notename, sizeof(out_notename), "%s", new_notename);
            out_flag = strdup(new_flag);
            // out_notename = strdup(new_notename);
            changed = 1;
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
        if (fprintf(fpw, "%s,%s,%s\n", out_flag, out_datetime, out_notename) < 0){
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
            return -1;
        }
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
        return -1;
    }

    if (fclose(fpr)){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (fclose(fpw)){
        perror(tmpfile);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (rename(tmpfile, list) != 0){
        perror("list rename");
        unlink(tmpfile);
        free(tmpfile);
        return -2;
    }
    
    free(tmpfile);

    if (changed == 0){
        return 1;
    }

    return 0;
}


// IO error, return -1
// if failed to copy tmporary file to list, return -2
// if flag is not found, return 1
// otherwise, return 0
int rm_key_in_list(const char* list, const char* target_flag){
    FILE* fpr;
    FILE* fpw;
    char* line;
    char* out_line;
    char* tmpfile;
    char* flag;
    char* datetime;
    char* notename;
    int   fd;
    int   removed;
    int   len;
    struct stat st;
    size_t size;

    removed = 0;

    if (stat(list, &st) != 0){
        perror(list);
        return -1;
    }

    len = strlen(list);
    tmpfile = malloc((len+8) * sizeof(char));
    snprintf(tmpfile, sizeof(tmpfile), "%s.XXXXXX", list);

    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (fchmod(fd, st.st_mode) != 0){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    while (getline(&line, &size, fpr) != -1){
        // replace '\n' to '\0'
        line[strcspn(line, "\n")] = '\0';

        // skip empty line
        if (is_white_space(line) == 1){
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
            return -1;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            removed = 1;
            continue;
        }

        // snprintf(out_line, sizeof(out_line), "%s,%s,%s\n", flag, datetime, notename);
        if (fprintf(fpw, "%s,%s,%s\n", flag, datetime, notename) < 0){
            perror(tmpfile);
            free(tmpfile);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            free(line);
            return -1;
        }
    }

    free(line);

    if (ferror(fpr)){
        perror(list);
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (fclose(fpr)){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (fclose(fpw)){
        perror(tmpfile);
        unlink(tmpfile);
        free(tmpfile);
        return -1;
    }

    if (rename(tmpfile, list) != 0){
        perror("list rename");
        unlink(tmpfile);
        free(tmpfile);
        return -2;
    }

    free(tmpfile);

    if (removed == 0){
        return 1;
    }

    return 0;
}


// return 0 if successed
// return 1 if input failed
// return 2 if line is white space
// return -1 if list file is broken: does not have thee elements
int get_content_line(FILE* fp, size_t flag_len, char* flag, size_t datetime_len, char* datetime, size_t notename_len, char* notename){
    char*  line;
    char*  in_flag;
    char*  in_datetime;
    char*  in_notename;
    char*  dummy;
    size_t size;

    if (getline(&line, &size, fp) != -1){
        return 1;
    }

    if (is_white_space(line)){
        free(line);
        return 2;
    }

    line[strcspn(line, "\n")] = '\0';

    in_flag     = strtok(line, DELIM);
    in_datetime = strtok(NULL, DELIM);
    in_notename = strtok(NULL, DELIM);
    dummy       = strtok(NULL, DELIM);

    if (in_flag == NULL || in_datetime == NULL || in_notename == NULL || dummy != NULL){
        free(line);
        return -1;
    }

    snprintf(flag    , flag_len    , "%s", in_flag    );
    snprintf(datetime, datetime_len, "%s", in_datetime);
    snprintf(notename, notename_len, "%s", in_notename);

    free(line);

    return 0;
}


