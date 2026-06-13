
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
// #include <strings.h>
#include <string.h>
#include <sys/stat.h>

#include "names.h"
#include "globals.h"

#define DATETIME_LEN 20
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


// col is zero-based
// if col=0 is specified, result is not overrided
int read_list_by_key(const char* list, char* target_flag, const int col, char* result){
    int i;
    FILE* fp;
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char* flag;

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return -2;
    }

    // read line by line
    while(fgets(line, sizeof(line), fp) != NULL){
        // replace '\n' to '\0'
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0'){
            continue;
        }

        // find the first delimiter
        flag = strtok(line, DELIM);

        #ifdef DEBUG
        printf("<DEBUG> Flag = %s\n", flag);
        #endif
        // if the flag of the current line is target_flag
        if (strcmp(flag, target_flag) == 0){
            if (col == 0){
                fclose(fp);
                return 0;
            }
            i = 1;
            while (flag != NULL){
                // find target column...
                strcpy(result, strtok(NULL, DELIM));
                #ifdef DEBUG
                printf("<DEBUG> Col = %d, Word = %s\n", i, result);
                #endif
                if (i == col){
                    #ifdef DEBUG
                    printf("<DEBUG> Extracted Word = %s\n", result);
                    #endif
                    fclose(fp);
                    return 0;
                }
                i = i + 1;
            }
            // if col exceeds the actual number of columns
            fprintf(stderr, "%s: Specified column is too large: %d", flag, 0);
            fclose(fp);
            exit(1);
        }
    }
    // if target_flag is not found
    fclose(fp);
    return -1;
}


// return 0 if flag does not exist
// return -1 if flag exist
int flag_exist_check(const char* list, char* flag){
    char dummy[128];
    int  stat;
    stat = read_list_by_key(list, flag, 0, dummy);

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
    stat = read_list_by_key(list, flag, 1, datetime);

    if (stat < 0){
        return -1;
    }
    return 0;
}


int get_filename_by_key(const char* list, char* flag, char* filename){
    int  stat;
    stat = read_list_by_key(list, flag, 2, filename);

    if (stat < 0){
        return -1;
    }
    return 0;
}


// IO error, return -1
// if failed to copy tmporary file to list, return -2
// if flag is not found, return 1
// otherwise, return 0
int mv_key_in_list(const char* list, const char* old_flag, const char* new_flag){
    FILE* fpr;
    FILE* fpw;
    char  line[FLAG_LEN+DATETIME_LEN+FILE_APATH_LEN+8];
    char  tmpfile[LIST_APATH_LEN+8];
    char* flag;
    char* datetime;
    char* notename;
    const char* out_flag;
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
        if (line[0] == '\0'){
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
            fprintf(stderr, "Invalid line format.\nFLAG = %s\nDATETIME = %s\nNOTENAME = %s\n", flag, datetime, notename);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            return -1;
        }

        out_flag = flag;
        // if the flag of the current line is target_flag
        if (strcmp(flag, old_flag) == 0){
            mv_filename(new_flag, sizeof(notename), notename);
            out_flag = new_flag;
            changed = 1;
        }

        snprintf(line, sizeof(line), "%s,%s,%s\n", out_flag, datetime, notename);
        if (fputs(line, fpw) == EOF){
            perror(tmpfile);
            fclose(fpr);
            fclose(fpw);
            unlink(tmpfile);
            return -1;
        }
        // if (write(fd, line, strlen(line)) == -1){
        //     fclose(fpr);
        //     close(fd);
        //     unlink(tmpfile);
        //     perror(tmpfile);
        //     return -1;
        // }
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
        if (line[0] == '\0'){
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
            fprintf(stderr, "Invalid line format.\nFLAG = %s\nDATETIME = %s\nNOTENAME = %s\n", flag, datetime, notename);
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


