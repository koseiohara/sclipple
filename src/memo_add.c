

#include <sys/stat.h>
#include <sys/types.h>
// #include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "globals.h"
#include "datetime.h"
#include "names.h"
#include "edit_list.h"


// if directory does not exist, run mkdir()
// if file, return -1
int make_dir(const char* dir){
    struct stat st;

    if (path_exist(dir)){
        #ifdef DEBUG
        printf("%s already exist\n", dir);
        #endif
        if (S_ISDIR(st.st_mode)){
            #ifdef DEBUG
            printf("%s is a directory\n", dir);
            #endif
            return 0;
        } else {
            #ifdef DEBUG
            printf("%s is NOT a directory\n", dir);
            #endif
            return -1;
        }
    } else {
        #ifdef DEBUG
        printf("mkdir %s\n", dir);
        #endif
        if (mkdir(dir, 0731) == -1){
            perror(dir);
            return -2;
        }
        return 0;
    }
}


// if file does not exist, open and close the specified file to make it
// if already exist, return -1
// if failed to open, return -2
int make_file(const char* path, const int cond){
    int fd;

    if (path_exist(path)){
        fd = open(path, cond, 0644);
        if (fd == -1){
            perror(path);
            return -2;
        }
        close(fd);
        return 0;
    } else {
        // perror(path);
        return -1;
    }
}


// if successfully added, return 0
// if flag is exist, return -1
// otherwise, stop process
int add(const char* list, const char* dir, const char* note_stock, char* flag, char* ext, struct tm* clock){
    char file[FILE_LEN];
    char path[FILE_APATH_LEN];
    char datetime[DATETIME_LEN];
    int  stat;

    #ifdef DEBUG
    printf("<DEBUG> list      : %s\n", list);
    printf("<DEBUG> dir       : %s\n", dir);
    printf("<DEBUG> note_stock: %s\n", note_stock);
    printf("<DEBUG> flag      : %s\n", flag);
    printf("<DEBUG> ext       : %s\n", ext);
    printf("<DEBUG> FILE_LEN      : %d\n", FILE_LEN);
    printf("<DEBUG> Length of file: %lu\n", strlen(file));
    #endif

    if (check_flag_length(flag) < 0){
        fprintf(stderr, "%s Error: Too long keyword: %s. Length should be less than %d", PROGRAM, flag, FLAG_LEN);
        return -1;
    }

    stat = make_dir(dir);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s Error: %s exists but is not a directory\n", PROGRAM, dir);
        }
        return -1;
    }

    stat = make_dir(note_stock);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s Error: %s exists but is not a directory\n", PROGRAM, dir);
        }
        return -1;
    }

    get_datetime(clock, '-', sizeof(datetime), datetime);
    get_filename(flag, datetime, ext, sizeof(file), file);
    snprintf(path, sizeof(path), "%s/%s", note_stock, file);
    #ifdef DEBUG
    printf("File name     : %s\n", file);
    printf("Note file name: %s\n", path);
    printf("List file name: %s\n", list);
    printf("Length of file: %lu\n", strlen(file));
    #endif

    stat = make_file(list, O_CREAT | O_WRONLY);
    if (stat == -2){
        return -1;
    }

    stat = flag_exist_check(list, flag);
    if (stat < 0){
        return stat;
    }
    #ifdef DEBUG
    printf("%s: Passed Flag Existence Check\n", flag);
    #endif

    stat = make_file(path, O_CREAT | O_EXCL | O_WRONLY);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s Error: %s already exists\n", PROGRAM, dir);
            return -1;
        } else if (stat == -2){
            fprintf(stderr, "%s Error: Failed to open %s\n", PROGRAM, dir);
            return -1;
        }
        fprintf(stderr, "%s Error: Undefined Error\n", PROGRAM);
        return -1;
    }

    get_datetime(clock, '-', sizeof(datetime), datetime);
    if (write_new_content_to_list(list, flag, datetime, path) == -1){
        return -1;
    }

    return 0;
}


