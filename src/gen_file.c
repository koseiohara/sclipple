

#include <sys/stat.h>
#include <sys/types.h>
#include <strings.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
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


int make_file(char* dir, char* file){
    struct stat st;
    char path[512];
    int fd;

    if (stat(file, &st) != 0){
        sprintf(path, "%s/%s", dir, file);
        fd = open(path, O_CREAT, 0644);
        if (fd == -1){
            perror("open");
        }
        close(fd);
        return 0;
    } else {
        return -1;
    }
}


void get_filename(char* dir, char* datetime, char* flag){
}


void add(char* dir, char* flag){
    if (make_dir(dir) == -1){
        fprintf(stderr, "%s exists but is not a directory\n", dir);
    }
    if (make_file(dir) == -1){
        fprintf(stderr, "%s exists but is not a directory\n", dir);
    }
}


