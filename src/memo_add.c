
#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>
// #include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "globals.h"
#include "ptrutils.h"
#include "datetime.h"
#include "file_systems.h"
#include "validation.h"
#include "list_utils.h"
#include "memo_add.h"


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
            return IO_ERROR;
        }
        close(fd);
        return 0;
    } else if (result == PATH_EXIST){
        return PATH_EXIST;
    } else{
        return ACCESS_FAILED_ERROR;
    }
}


// return INVALID_KEY_ERROR if keys is invalid
// return IO_ERROR if make directory and make file failed
// return MALLOC_ERROR if malloc failed
// return KEY_DUPLICATE if keyword already exist
// return LIST_FORMAT_ERROR if list file is broken
// return PATH_EXIST if note file already exist
// return UNKONWN_ERROR when bug
// return 0 otherwise
int add(const char* list, const char* dir, const char* note_stock, int nkeys, char** keys, int ntags, char** tags, char* ext, struct tm* clock){
    FILE* fp = NULL;
    char* file     = NULL;
    char* path     = NULL;
    char* datetime = NULL;
    char** exists  = NULL;
    char** nexists = NULL;
    int   result;
    int   ret;
    int   nexist_count;
    int   i;

    // Key validation
    for (i = 0; i < nkeys; i = i + 1){
        if (keys[i] == NULL){
            fprintf(stderr, "%s: Unknown Error: keys[%d] is NULL\n", PACKAGE_NAME, i);
            ret = UNKNOWN_ERROR;
            goto cleanup;
        }

        result = key_validation(keys[i]);
        if (result != 0){
            if (result == INPUT_ERROR){
                fprintf(stderr, "%s: Keyword is empty\n", PACKAGE_NAME);
            } else if (result == CHARACTER_NOT_ALLOWED_ERROR){
                fprintf(stderr, "%s: Invalid character is included in '%s'. Keywords can include alphabets, numbers, '_', and '-'\n", PACKAGE_NAME, keys[i]);
            } else if (result == RESERVED_WORD_ERROR){
                fprintf(stderr, "%s: '%s' is a reserved word.\n", PACKAGE_NAME, keys[i]);
            }
            ret = INVALID_KEY_ERROR;
            goto cleanup;
        }
    }

    // Tag validation
    for (i = 0; i < ntags; i = i + 1){
        if (tags[i] == NULL){
            fprintf(stderr, "%s: Unknown Error: tags[%d] is NULL\n", PACKAGE_NAME, i);
            ret = UNKNOWN_ERROR;
            goto cleanup;
        }

        result = tag_validation(tags[i]);
        if (result != 0){
            if (result == INPUT_ERROR){
                fprintf(stderr, "%s: Tag is empty\n", PACKAGE_NAME);
            } else if (result == CHARACTER_NOT_ALLOWED_ERROR){
                fprintf(stderr, "%s: Invalid character is included in a tag '%s'. Tags can include alphabets, numbers, '_', '-', and '.'\n", PACKAGE_NAME, keys[i]);
            } else if (result == RESERVED_WORD_ERROR){
                fprintf(stderr, "%s: '%s' is a reserved word.\n", PACKAGE_NAME, keys[i]);
            }
            ret = INVALID_KEY_ERROR;
            goto cleanup;
        }
    }
    if (tags[ntags] != NULL){
        fprintf(stderr, "%s: Unknown Error: tags[%d] is not NULL\n", PACKAGE_NAME, ntags);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }


    result = make_dir(dir);
    if (result != 0 && result != IS_DIRECTORY){
        if (result == IS_NOT_DIRECTORY_ERROR){
            fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, dir);
        } else if (result == MKDIR_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, dir, strerror(errno));
        }
        ret = IO_ERROR;
        goto cleanup;
    }

    result = make_dir(note_stock);
    if (result != 0 && result != IS_DIRECTORY){
        if (result == IS_NOT_DIRECTORY_ERROR){
            fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, note_stock);
        } else if (result == MKDIR_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, note_stock, strerror(errno));
        }
        ret = IO_ERROR;
        goto cleanup;
    }

    result = make_file(list, O_CREAT | O_WRONLY);
    if (result == IO_ERROR || result == ACCESS_FAILED_ERROR){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }

    if (nkeys <= 0){
        fprintf(stderr, "%s: Unknown error: No keys were speicified to add\n", PACKAGE_NAME);
        ret = INPUT_ERROR;
        goto cleanup;
    }

    result = get_datetime(clock, '\0', &datetime);
    if (result == MALLOC_ERROR){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    exists  = malloc((size_t)nkeys * sizeof(char*));
    nexists = malloc((size_t)nkeys * sizeof(char*));
    if (exists == NULL || nexists == NULL){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    ret = 0;

    fp = fopen(list, "a+");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    rewind(fp);
    result = key_exist_check(fp, nkeys, keys, exists, nexists);
    if (result == KEY_NOT_FOUND || result == 0){
        for (i = 0; i < nkeys && exists[i] != NULL; i = i + 1){
            fprintf(stderr, "%s: Key '%s' already exists\n", PACKAGE_NAME, exists[i]);
        }
        // i = 0;
        // while (exists[i] != NULL){
        //     fprintf(stderr, "%s: Key '%s' already exists\n", PACKAGE_NAME, exists[i]);
        //     i = i + 1;
        // }
        if (result == 0){
            ret = KEY_DUPLICATE;
            goto cleanup;
        }
    } else{
        if (result == IO_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, path, strerror(errno));
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
        } else{
            fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    nexist_count = 0;
    if (nexists[nkeys-1] != NULL){
        nexist_count = nkeys;
    } else{
        for (i = 0; i < nkeys && nexists[i] != NULL; i = i + 1){
            nexist_count = nexist_count + 1;
        }
    }

    for (i = 0; i < nexist_count; i = i + 1){
        result = get_filename(nexists[i], ext, &file);
        if (result == MALLOC_ERROR){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }

        result = asprintf(&path, "%s/%s", note_stock, file);
        if (result < 0){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            ret = MALLOC_ERROR;
            goto cleanup;
        }

        result = make_file(path, O_CREAT | O_EXCL | O_WRONLY);
        if (result == IO_ERROR || result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, path, strerror(errno));
            ret = IO_ERROR;
            goto cleanup;
        } else if (result == PATH_EXIST){
            fprintf(stderr, "%s: %s already exists\n", PACKAGE_NAME, path);
            ret = PATH_EXIST;
            goto cleanup;
        }

        result = add_contents_to_list(fp, nexists[i], path, datetime, ntags, tags);
        if (result != 0){
            if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
                ret = IO_ERROR;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
            } else{
                fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                ret = UNKNOWN_ERROR;
            }

            if (unlink(path) != 0){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, path, strerror(errno));
            }
            goto cleanup;
        }

        fprintf(stdout, "%s: New note was created: '%s'\n", PACKAGE_NAME, nexists[i]);

        XFREE(file);
        XFREE(path);
    }
    // ret = 0;
    fprintf(stdout, "%s: %d keys were created\n", PACKAGE_NAME, nexist_count);
    goto cleanup;


cleanup:
    xfclose(&fp);
    free(datetime);
    free(path);
    free(file);
    free(exists);
    free(nexists);

    return ret;
}


