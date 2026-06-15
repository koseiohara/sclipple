
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
    int fd;
    // char list_path[DIR_LEN+FILE_LEN+1];
    char content[FLAG_LEN+1+DATETIME_LEN+1+FILE_APATH_LEN+16];

    fd = open(list, O_WRONLY | O_APPEND);
    if (fd == -1){
        perror(list);
        return -1;
    }
    snprintf(content, sizeof(content), "%s,%s,%s\n", flag, datetime, file);

    #ifdef DEBUG
    printf("%s\n", content);
    #endif
    if (write(fd, content, strlen(content)) == -1){
        perror(list);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}


int read_list_by_key(FILE* fp, char* target_flag, const int col, size_t result_len, char* result){
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char* flag;
    int   i;

    while(fgets(line, sizeof(line), fp) != NULL){
        line[strcspn(line, "\n")] = '\0';

        if (is_white_space(line) == 1){
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
}


// col is zero-based
// if col=0 is specified, result is not overrided
int read_list_by_key_old(FILE* fp, char* target_flag, const int col, char* result){
    int i;
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char* flag;
    char* s;

    // read line by line
    while(fgets(line, sizeof(line), fp) != NULL){
        // replace '\n' to '\0'
        line[strcspn(line, "\n")] = '\0';

        if (is_white_space(line) == 1){
            continue;
        }

        // find the first delimiter
        flag = strtok(line, DELIM);
        if (flag == NULL){
            fprintf(stderr, "%s Error: Invalid list file. list file is broken\n", PROGRAM);
            return -2;
        }

        #ifdef DEBUG
        printf("<DEBUG> Flag = %s\n", flag);
        #endif
        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            if (col == 0){
                // fclose(fp);
                return 0;
            }
            i = 1;
            while (true){
                // find target column...
                s = strtok(NULL, DELIM);
                if (s == NULL){
                    fprintf(stderr, "%s Error: Invalid list file. list file is broken\n", PROGRAM);
                    return -2;
                }
                strcpy(result, s);
                #ifdef DEBUG
                printf("<DEBUG> Col = %d, Word = %s\n", i, result);
                #endif
                if (i == col){
                    #ifdef DEBUG
                    printf("<DEBUG> Extracted Word = %s\n", result);
                    #endif
                    // fclose(fp);
                    return 0;
                }
                i = i + 1;
            }
            // if col exceeds the actual number of columns
            fprintf(stderr, "%s: %s: Specified column is too large: %d", PROGRAM, flag, 0);
            fclose(fp);
            exit(1);
        }
    }
    // if target_flag is not found
    // fclose(fp);
    return -1;
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
        fclose(fp);
        return -2;
    }

    stat = read_list_by_key(fp, flag, 0, dummy);

    fclose(fp);

    if (stat == -1){
        return 0;
    } else if (stat == 0){
        return -1;
    } else if (stat == -2){
        exit(1);
    }
    return -2;
}


int get_datetime_by_key(const char* list, char* flag, char* datetime){
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        fclose(fp);
        return -2;
    }

    stat = read_list_by_key(fp, flag, 1, datetime);

    fclose(fp);

    if (stat < 0){
        return -1;
    }
    return 0;
}


int get_filename_by_key(const char* list, char* flag, char* filename){
    int  stat;
    FILE* fp;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        fclose(fp);
        return -2;
    }

    stat = read_list_by_key(fp, flag, 2, filename);

    fclose(fp);

    if (stat < 0){
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
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char  tmpfile[LIST_APATH_LEN+8];
    char* flag;
    char* datetime;
    char* notename;
    char  new_notename[FILE_APATH_LEN];
    char  out_flag[FLAG_LEN];
    char  out_datetime[DATETIME_LEN];
    char  out_notename[FILE_APATH_LEN];
    char  dummy[128];
    // const char* out_flag;
    int   fd;
    int   changed;
    struct stat st;

    changed = 0;

    if (stat(list, &st) != 0){
        perror(list);
        return -1;
    }


    snprintf(tmpfile, sizeof(tmpfile), "%s.XXXXXX", list);
    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        return -1;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        return -1;
    }

    if (fchmod(fd, st.st_mode) != 0){
        perror(tmpfile);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    // check existence of the new flag
    if (read_list_by_key(fpr, new_flag, 0, dummy) != -1){
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
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
    while (fgets(line, sizeof(line), fpr) != NULL){
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
        printf("<DEBUG> FLAG    : %s\n", flag);
        printf("<DEBUG> DATETIME: %s\n", datetime);
        printf("<DEBUG> NOTENAME: %s\n", notename);
        #endif

        if (flag == NULL || datetime == NULL || notename == NULL){
            fprintf(stderr, "%s: Invalid line format.\nFLAG = %s\nDATETIME = %s\nNOTENAME = %s\n", PROGRAM, flag, datetime, notename);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            return -1;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, old_flag) == 0){
            if (mv_filename(notename, new_flag, sizeof(new_notename), new_notename) < 0){
                return -3;
            }
            snprintf(out_flag    , sizeof(out_flag)    , "%s", new_flag);
            snprintf(out_notename, sizeof(out_notename), "%s", new_notename);
            changed = 1;
        } else{
            snprintf(out_flag    , sizeof(out_flag)    , "%s", flag);
            snprintf(out_notename, sizeof(out_notename), "%s", notename);
        }
        snprintf(out_datetime, sizeof(out_datetime), "%s", datetime);

        snprintf(line, sizeof(line), "%s,%s,%s\n", out_flag, out_datetime, out_notename);
        if (fputs(line, fpw) == EOF){
            perror(tmpfile);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            return -1;
        }
    }

    if (ferror(fpr)){
        perror(list);
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    if (fclose(fpr)){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    if (fclose(fpw)){
        perror(tmpfile);
        unlink(tmpfile);
        return -1;
    }

    if (rename(tmpfile, list) != 0){
        perror("list rename");
        unlink(tmpfile);
        return -2;
    }
    
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
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char  tmpfile[LIST_APATH_LEN+8];
    char* flag;
    char* datetime;
    char* notename;
    int   fd;
    int   removed;
    struct stat st;

    removed = 0;

    if (stat(list, &st) != 0){
        perror(list);
        return -1;
    }


    snprintf(tmpfile, sizeof(tmpfile), "%s.XXXXXX", list);
    fd = mkstemp(tmpfile);
    if (fd == -1){
        perror(tmpfile);
        return -1;
    }

    if (fchmod(fd, st.st_mode) != 0){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        return -1;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        perror(tmpfile);
        close(fd);
        unlink(tmpfile);
        return -1;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    while (fgets(line, sizeof(line), fpr) != NULL){
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
            return -1;
        }

        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            removed = 1;
            continue;
        }

        snprintf(line, sizeof(line), "%s,%s,%s\n", flag, datetime, notename);
        if (fputs(line, fpw) == EOF){
            perror(tmpfile);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            return -1;
        }
    }

    if (ferror(fpr)){
        perror(list);
        fclose(fpr);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    if (fclose(fpr)){
        perror(list);
        fclose(fpw);
        unlink(tmpfile);
        return -1;
    }

    if (fclose(fpw)){
        perror(tmpfile);
        unlink(tmpfile);
        return -1;
    }

    if (rename(tmpfile, list) != 0){
        perror("list rename");
        unlink(tmpfile);
        return -2;
    }

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
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char* in_flag;
    char* in_datetime;
    char* in_notename;
    char* dummy;

    if (fgets(line, sizeof(line), fp) == NULL){
        return 1;
    }

    if (is_white_space(line)){
        return 2;
    }

    line[strcspn(line, "\n")] = '\0';

    in_flag     = strtok(line, DELIM);
    in_datetime = strtok(NULL, DELIM);
    in_notename = strtok(NULL, DELIM);
    dummy       = strtok(NULL, DELIM);

    if (in_flag == NULL || in_datetime == NULL || in_notename == NULL || dummy != NULL){
        return -1;
    }

    snprintf(flag    , flag_len    , "%s", in_flag    );
    snprintf(datetime, datetime_len, "%s", in_datetime);
    snprintf(notename, notename_len, "%s", in_notename);

    return 0;
}


