

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
int make_dir(char* dir){
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
int make_file(char* path, int cond){
    struct stat st;
    int fd;

    if (stat(path, &st) != 0){
        fd = open(path, cond, 0644);
        if (fd == -1){
            perror("open");
            return -1;
        }
        close(fd);
        return 0;
    } else {
        perror("open");
        return -1;
    }
}


// if failed to open list file or failed to write info to list, return -1
// otherwise, return 0
int add_to_list(char* list, char* flag, char* datetime, char* file){
    int fd;
    // char list_path[DIR_LEN+FILE_LEN+1];
    char content[FLAG_LEN+DIR_LEN+FILE_LEN+24];

    fd = open(list, O_WRONLY | O_APPEND);
    if (fd == -1){
        perror(list);
        return -1;
    }
    sprintf(content, "%s,%s,%s", flag, datetime, file);

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
int add(char* list, char* dir, char* subdir, char* flag){
    char file[FILE_LEN];
    char path[DIR_LEN+SUBDIR_LEN+FILE_LEN+2];
    char list_path[DIR_LEN+LIST_LEN+1];
    char datetime[20];
    int  stat;

    stat = make_dir(dir);
    if (stat < 0){
        if (stat == -1){
            fprintf(stderr, "%s exists but is not a directory\n", dir);
        }
        exit(1);
    }

    get_filename(flag, file);
    sprintf(path, "%s/%s/%s", dir, subdir, file);
    sprintf(list_path, "%s/%s", dir, list);
    #ifdef DEBUG
    printf("Note file name: %s\n", path);
    printf("List file name: %s\n", list_path);
    #endif

    if (make_file(path, O_CREAT | O_EXCL | O_WRONLY) < 0){
        // fprintf(stderr, "%s exists but is not a directory\n", dir);
        exit(1);
    }

    stat = make_file(list_path, O_CREAT | O_WRONLY);
    if (stat == -1){
        return -1;
    } else if (stat == -2){
        exit(1);
    } else{
        flag_exist_check(list_path, flag);
    }

    get_datetime(datetime);
    if (add_to_list(list_path, flag, datetime, path) == -1){
        exit(1);
    }

    return 0;
}


