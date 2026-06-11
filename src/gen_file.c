

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


int make_dir(char* dir){
    struct stat st;

    if (stat(dir, &st) == 0){
        if (S_ISDIR(st.st_mode)){
            return 0;
        } else {
            return -1;
        }
    } else {
        mkdir(dir, 0731);
        return 0;
    }
}


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


void get_filename(char* flag, char* output){
    sprintf(output, "%s.txt", flag);
}


int add_to_list(char* list, char* flag, char* datetime, char* file){
    int fd;
    char list_path[DIR_LEN+FILE_LEN+1];
    char content[FLAG_LEN+DIR_LEN+FILE_LEN+24];

    // sprintf(list_path, "%s/%s", list_dir, list_file);
    if (make_file(list, O_CREAT | O_WRONLY) == -1){
        // fprintf(stderr, "%s exists but is not a directory\n", dir);
        return -1;
    }

    fd = open(list, O_WRONLY | O_APPEND);
    if (fd == -1){
        perror(list);
        return -1;
    }
    sprintf(content, "%s %s %s", flag, datetime, file);

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


void add(char* list, char* dir, char* flag){
    char file[FILE_LEN];
    char path[DIR_LEN+FILE_LEN+1];
    char datetime[20];

    if (make_dir(dir) == -1){
        fprintf(stderr, "%s exists but is not a directory\n", dir);
        exit(1);
    }

    get_filename(flag, file);
    sprintf(path, "%s/%s", dir, file);
    #ifdef DEBUG
    printf("Note file name: %s\n", path);
    #endif

    if (make_file(path, O_CREAT | O_EXCL | O_WRONLY) == -1){
        // fprintf(stderr, "%s exists but is not a directory\n", dir);
        exit(1);
    }

    get_datetime(datetime);
    if (add_to_list(list, flag, datetime, path) == -1){
        exit(1);
    }
}


