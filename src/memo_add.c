

#include <sys/stat.h>
#include <sys/types.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include "globals.h"
#include "datetime.h"
#include "filename.h"
#include "read_list.h"


// if directory does not exist, run mkdir()
// if file, return -1
int make_dir(const char* dir){
    struct stat st;

    if (stat(dir, &st) == 0){
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
    struct stat st;
    int fd;

    if (stat(path, &st) != 0){
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


// if failed to open list file or failed to write info to list, return -1
// otherwise, return 0
int add_to_list(const char* list, const char* flag, const char* datetime, const char* file){
    int fd;
    // char list_path[DIR_LEN+FILE_LEN+1];
    char content[FLAG_LEN+DIR_LEN+FILE_LEN+24];

    fd = open(list, O_WRONLY | O_APPEND);
    if (fd == -1){
        perror(list);
        return -1;
    }
    sprintf(content, "%s,%s,%s\n", flag, datetime, file);

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


// if successfully added, return 0
// if flag is exist, return -1
// otherwise, stop process
int add(const char* list, const char* dir, const char* note_stock, char* flag){
    char file[FILE_LEN];
    char path[DIR_LEN+SUBDIR_LEN+FILE_LEN+2];
    char list_path[DIR_LEN+LIST_LEN+1];
    char datetime[20];
    int  stat;

    if (strlen(flag) >= FLAG_LEN){
        fprintf(stderr, "Too long keyword: %s. Length should be less than %d", flag, FLAG_LEN);
        exit(1);
    }

    stat = make_dir(dir);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s exists but is not a directory\n", dir);
        }
        exit(1);
    }

    stat = make_dir(note_stock);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s exists but is not a directory\n", dir);
        }
        exit(1);
    }

    get_filename(flag, file);
    sprintf(path, "%s/%s", note_stock, file);
    sprintf(list_path, "%s/%s", dir, list);
    #ifdef DEBUG
    printf("Note file name: %s\n", path);
    printf("List file name: %s\n", list_path);
    #endif

    stat = make_file(list_path, O_CREAT | O_WRONLY);
    if (stat == -2){
        exit(1);
    }

    stat = flag_exist_check(list_path, flag);
    if (stat < 0){
        return stat;
    }
    #ifdef DEBUG
    printf("%s: Passed Flag Existence Check\n", flag);
    #endif

    stat = make_file(path, O_CREAT | O_EXCL | O_WRONLY);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s is exists\n", dir);
            exit(1);
        } else if (stat == -2){
            fprintf(stderr, "Failed to open %s\n", dir);
            exit(1);
        }
        fprintf(stderr, "Undefined Error\n");
        exit(1);
    }

    get_datetime(datetime);
    if (add_to_list(list_path, flag, datetime, path) == -1){
        exit(1);
    }

    return 0;
}


