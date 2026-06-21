
#include "config.h"

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


#define IS_NOT_DIRECTORY_ERROR -1
#define MKDIR_ERROR -2
#define IS_DIRECTORY 1

// if directory does not exist, run mkdir()
// return IS_DIRECTORY if dir already exist and is a directory
// return IS_NOT_DIRECTORY_ERROR if dir already exist and is not a directory
// return MKDIR_ERROR if mkdir() failed
// return ACCESS_FAILED_ERROR if failed to access dir
int make_dir(const char* dir){
    struct stat st;
    int result;

    result = path_status(dir, &st);
    if (result == PATH_EXIST){
        #ifdef DEBUG
        printf("%s already exist\n", dir);
        #endif
        if (S_ISDIR(st.st_mode)){
            #ifdef DEBUG
            printf("%s is a directory\n", dir);
            #endif
            return IS_DIRECTORY;
        } else {
            #ifdef DEBUG
            printf("%s is NOT a directory\n", dir);
            #endif
            return IS_NOT_DIRECTORY_ERROR;
        }
    } else if (result == PATH_NOT_EXIST){
        #ifdef DEBUG
        printf("mkdir %s\n", dir);
        #endif
        if (mkdir(dir, 0755) == -1){
            perror(dir);
            return MKDIR_ERROR;
        }
        return 0;
    } else{
        return ACCESS_FAILED_ERROR;
    }
}


// if file does not exist, open and close the specified file to make it
// return IO_ERROR if failed to open
// return PATH_EXIST if path already exist
// return ACCESS_FAILED_ERROR if failed to access path
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR if program has bug
// return 0 and make a file if file does not exist
int make_file(const char* path, const int cond){
    struct stat st;
    int result;
    int fd;

    result = path_status(path, &st);
    if (result == PATH_NOT_EXIST){
        fd = open(path, cond, 0644);
        if (fd == -1){
            perror(path);
            return IO_ERROR;
        }
        close(fd);
        return 0;
    } else if (result == PATH_EXIST){
        return PATH_EXIST;
    } else{
        fprintf(stderr, "%s: Failed to open %s\n", PACKAGE_NAME, path);
        return ACCESS_FAILED_ERROR;
    }
}


// return INVALID_KEY_ERROR if flag is invalid
// return IO_ERROR if make directory and make file failed
// return MALLOC_ERROR if malloc failed
// return KEY_DUPLICATE if keyword already exist
// return PATH_EXIST if note file already exist
// return 0 otherwise
int add(const char* list, const char* dir, const char* note_stock, char* flag, char* ext, struct tm* clock){
    char* file     = NULL;
    char* path     = NULL;
    char* datetime = NULL;
    int   result;
    // int   len;

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
        if (result == INPUT_ERROR){
            fprintf(stderr, "%s: Keyword is empty\n", PACKAGE_NAME);
        } else if (result == CHARACTER_NOT_ALLOWED_ERROR){
            fprintf(stderr, "%s: Invalid character is included in '%s'. Keywords can include alphabets, numbers, '_', and '-'\n", PACKAGE_NAME, flag);
        } else if (result == RESERVED_WORD_ERROR){
            fprintf(stderr, "%s: '%s' is a reserved word.\n", PACKAGE_NAME, flag);
        }
        return INVALID_KEY_ERROR;
    }

    result = make_dir(dir);
    if (result < 0){
        if (result == IS_NOT_DIRECTORY_ERROR){
            fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, dir);
        } else if (result == MKDIR_ERROR){
            fprintf(stderr, "%s: Failed to make directory '%s'\n", PACKAGE_NAME, dir);
        }
        return IO_ERROR;
    }

    result = make_dir(note_stock);
    if (result < 0){
        if (result == IS_NOT_DIRECTORY_ERROR){
            fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, dir);
        } else if (result == MKDIR_ERROR){
            fprintf(stderr, "%s: Failed to make directory '%s'\n", PACKAGE_NAME, dir);
        }
        return IO_ERROR;
    }

    result = get_datetime(clock, '-', &datetime);
    if (result == MALLOC_ERROR){
        return MALLOC_ERROR;
    }
    result = get_filename(flag, datetime, ext, &file);
    if (result == MALLOC_ERROR){
        return MALLOC_ERROR;
    }

    result = asprintf(&path, "%s/%s", note_stock, file);
    if (result < 0){
        perror("asprintf");
        return MALLOC_ERROR;
    }
    #ifdef DEBUG
    printf("File name     : %s\n", file);
    printf("Note file name: %s\n", path);
    printf("List file name: %s\n", list);
    printf("Length of file: %lu\n", strlen(file));
    #endif

    result = make_file(list, O_CREAT | O_WRONLY);
    if (result == IO_ERROR || result == ACCESS_FAILED_ERROR){
        // fprintf(stderr, "%s Error: Failed to make list file\n", PACKAGE_NAME);
        free(datetime);
        free(file);
        free(path);
        return IO_ERROR;
    }

    result = flag_exist_check(list, flag);
    if (result == true){
        fprintf(stderr, "%s: Keyword '%s' already exists\n", PACKAGE_NAME, flag);
        free(datetime);
        free(file);
        free(path);
        return KEY_DUPLICATE;
    } else if (result < 0){
        free(datetime);
        free(file);
        free(path);
        if (result == LIST_FORMAT_ERROR){
            fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
            return LIST_FORMAT_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
            return UNKNOWN_ERROR;
        }
    }
    #ifdef DEBUG
    printf("%s: Passed Flag Existence Check\n", flag);
    #endif

    result = make_file(path, O_CREAT | O_EXCL | O_WRONLY);
    if (result < 0){
        if (result == IO_ERROR){
            fprintf(stderr, "%s: Failed to open %s\n", PACKAGE_NAME, path);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access to %s\n", PACKAGE_NAME, path);
        } else{
            fprintf(stderr, "%s: Undefined Error\n", PACKAGE_NAME);
        }
        free(datetime);
        free(file);
        free(path);
        return IO_ERROR;
    } else if (result == PATH_EXIST){
        fprintf(stderr, "%s: %s already exists\n", PACKAGE_NAME, path);
        free(datetime);
        free(file);
        free(path);
        return PATH_EXIST;
    } else{
        printf("%s: Create new note: '%s'\n", PACKAGE_NAME, flag);
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
        return IO_ERROR;
    }

    free(datetime);
    free(path);
    free(file);

    return 0;
}


