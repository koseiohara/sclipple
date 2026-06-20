

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


// if directory does not exist, run mkdir() and return 0
// if directory already exist, return 1
// if file, return -1
// if mkdir failed, return -2
int make_dir(const char* dir){
    struct stat st;
    int result;

    result = path_status(dir, &st);
    if (result == 1){
        #ifdef DEBUG
        printf("%s already exist\n", dir);
        #endif
        if (S_ISDIR(st.st_mode)){
            #ifdef DEBUG
            printf("%s is a directory\n", dir);
            #endif
            return 1;
        } else {
            #ifdef DEBUG
            printf("%s is NOT a directory\n", dir);
            #endif
            return -1;
        }
    } else if (result == 0){
        #ifdef DEBUG
        printf("mkdir %s\n", dir);
        #endif
        if (mkdir(dir, 0755) == -1){
            perror(dir);
            return -2;
        }
        return 0;
    } else{
        return -1;
    }
}


// if file does not exist, open and close the specified file to make it
// if file does not exist, run open and return 0
// if already exist, return -1
// if failed to open, return -2
int make_file(const char* path, const int cond){
    struct stat st;
    int result;
    int fd;

    result = path_status(path, &st);
    if (result == 0){
        fd = open(path, cond, 0644);
        if (fd == -1){
            perror(path);
            return -2;
        }
        close(fd);
        return 0;
    } else if (result == 1){
        return -1;
    } else{
        fprintf(stderr, "%s IO Error: Failed to open %s\n", PROGRAM, path);
        return -2;
    }
}


// if successfully added, return 0
// if flag is exist, return -1
// if IO error, return -2
// if flag includes invalid character or is too long, return -3
// otherwise, stop process
int add(const char* list, const char* dir, const char* note_stock, char* flag, char* ext, struct tm* clock){
    char* file     = NULL;
    char* path     = NULL;
    char* datetime = NULL;
    int   result;
    int   len;

    #ifdef DEBUG
    printf("<DEBUG> list      : %s\n", list);
    printf("<DEBUG> dir       : %s\n", dir);
    printf("<DEBUG> note_stock: %s\n", note_stock);
    printf("<DEBUG> flag      : %s\n", flag);
    printf("<DEBUG> ext       : %s\n", ext);
    printf("<DEBUG> Length of file: %lu\n", strlen(file));
    #endif

    result = flag_validation(flag);
    if (result < 0){
        if (result == -1){
            fprintf(stderr, "%s Error: Keyword is empty\n", PROGRAM);
        } else if (result == -2){
            fprintf(stderr, "%s Error: Invalid character is included in '%s'. Keywords can include alphabets, numbers, '_', and '-'\n", PROGRAM, flag);
        } else if (result == -3){
            fprintf(stderr, "%s Error: '%s' is a reserved word.\n", PROGRAM, flag);
        }
        return -3;
    }

    result = make_dir(dir);
    if (result < 0){
        if (result == -1){
            fprintf(stderr, "%s Error: %s exists but is not a directory\n", PROGRAM, dir);
        }
        return -2;
    } else if (result == 0){
        printf("%s: Running initialization process\n", PROGRAM);
    }

    result = make_dir(note_stock);
    if (result < 0){
        if (result == -1){
            fprintf(stderr, "%s Error: %s exists but is not a directory\n", PROGRAM, dir);
        }
        return -2;
    } else if (result == 0){
        printf("%s: Running initialization process\n", PROGRAM);
    }

    result = get_datetime(clock, '-', &datetime);
    if (result == MALLOC_ERROR){
        return MALLOC_ERROR;
    }
    get_filename(flag, datetime, ext, &file);

    // len = 2;        // for slash and \0
    // len = len + strlen(note_stock);
    // len = len + strlen(file);
    // path = malloc(len * sizeof(char));
    // snprintf(path, len, "%s/%s", note_stock, file);
    result = asprintf(&path, "%s/%s", note_stock, file);
    if (result == MALLOC_ERROR){
        return MALLOC_ERROR;
    }
    #ifdef DEBUG
    printf("File name     : %s\n", file);
    printf("Note file name: %s\n", path);
    printf("List file name: %s\n", list);
    printf("Length of file: %lu\n", strlen(file));
    #endif

    result = make_file(list, O_CREAT | O_WRONLY);
    if (result == -2){
        // fprintf(stderr, "%s Error: Failed to make list file\n", PROGRAM);
        free(datetime);
        free(file);
        free(path);
        return -2;
    } else if (result == 0){
        printf("%s: Running initialization process\n", PROGRAM);
    }

    result = flag_exist_check(list, flag);
    if (result < 0){
        if (result == -1){
            fprintf(stderr, "%s: '%s' already exist\n", PROGRAM, flag);
            free(datetime);
            free(file);
            free(path);
            return -1;
        } else if (result == -2){
            free(datetime);
            free(file);
            free(path);
            return -2;
        }
        free(datetime);
        free(file);
        free(path);
        return -4;
    }
    #ifdef DEBUG
    printf("%s: Passed Flag Existence Check\n", flag);
    #endif

    result = make_file(path, O_CREAT | O_EXCL | O_WRONLY);
    if (result < 0){
        if (result == -1){
            fprintf(stderr, "%s Error: %s already exists\n", PROGRAM, path);
            free(datetime);
            free(file);
            free(path);
            return -1;
        } else if (result == -2){
            // fprintf(stderr, "%s IO Error: Failed to open %s\n", PROGRAM, path);
            free(datetime);
            free(file);
            free(path);
            return -2;
        }
        fprintf(stderr, "%s Error: Undefined Error\n", PROGRAM);
        free(datetime);
        free(file);
        free(path);
        return -4;
    } else{
        printf("%s: Create new note: '%s'\n", PROGRAM, flag);
    }

    free(datetime);
    datetime = NULL;

    result = get_datetime(clock, '\0', &datetime);
    if (result == MALLOC_ERROR){
        return MALLOC_ERROR;
    }
    if (write_new_content_to_list(list, flag, datetime, path) == IO_ERROR){
        free(datetime);
        free(path);
        free(file);
        return -1;
    }

    free(datetime);
    free(path);
    free(file);

    return 0;
}


