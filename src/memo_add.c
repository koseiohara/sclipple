
#include "config.h"

// #include <sys/stat.h>
// #include <sys/types.h>
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
#include "strutils.h"
#include "datetime.h"
#include "file_systems.h"
#include "validation.h"
#include "list_utils.h"
#include "memo_add.h"


// return IO_ERROR if dir or note_stock is not a directory, make-directory failed, make-file failed, or access to list file failed
// return 0 if list file already exists or initialization completed
int file_init(const char* dir, const char* note_stock, const char* list){
    struct stat st;
    int result;

    // check the existence of the list file
    result = path_status(list, &st);
    if (result == PATH_EXIST){
        return 0;
    }

    if (result == ACCESS_FAILED_ERROR){
        result = path_status(dir, &st);
        if (result != PATH_EXIST){
            fprintf(stderr, "%s: Failed to access %s\n", PACKAGE_NAME, dir);
            return IO_ERROR;
        }

        if (!S_ISDIR(st.st_mode)){
            fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, dir);
            return IO_ERROR;
        }

        fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        return IO_ERROR;
    }

    if (result == PATH_NOT_EXIST){
        fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        fprintf(stderr, "%s: Started initialization processes\n", PACKAGE_NAME);

        result = make_dir(dir);
        if (result != 0 && result != IS_DIRECTORY){
            if (result == IS_NOT_DIRECTORY_ERROR){
                fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, dir);
            } else if (result == MKDIR_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, dir, strerror(errno));
            }
            return IO_ERROR;
        }

        result = make_dir(note_stock);
        if (result != 0 && result != IS_DIRECTORY){
            if (result == IS_NOT_DIRECTORY_ERROR){
                fprintf(stderr, "%s: '%s' exists but is not a directory\n", PACKAGE_NAME, note_stock);
            } else if (result == MKDIR_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, note_stock, strerror(errno));
            }
            return IO_ERROR;
        }

        result = make_file(list, O_CREAT | O_WRONLY);
        if (result == IO_ERROR || result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            return IO_ERROR;
        }

        fprintf(stderr, "%s: Completed\n", PACKAGE_NAME);
        return 0;
    }

    return IO_ERROR;
}


// return INVALID_KEY_ERROR if keys are invalid
// return INVALID_TAG_ERROR if tags are invalid
// return IO_ERROR if make directory and make file failed
// return MALLOC_ERROR if malloc failed
// return KEY_DUPLICATE if keyword already exist
// return LIST_FORMAT_ERROR if list file is broken
// return PATH_EXIST if note file already exist
// return UNKONWN_ERROR when bug
// return 0 otherwise
int add(const char* list, const char* dir, const char* note_stock, int nkeys, char** keys, int ntags, char** tags, char* ext, struct tm* clock){
    FILE* fp       = NULL;
    char* file     = NULL;
    char* path     = NULL;
    char* datetime = NULL;
    char** exists  = NULL;
    char** nexists = NULL;
    int   result;
    int   ret;
    int   nexist_count;
    int   i;

    if (nkeys <= 0 || ntags < 0){
        if (nkeys <= 0){
            fprintf(stderr, "%s: Unknown error: No keys were specified to add\n", PACKAGE_NAME);
        } else{
            fprintf(stderr, "%s: Unknown error: Number of tags is negative\n", PACKAGE_NAME);
        }
        ret = INPUT_ERROR;
        goto cleanup;
    }

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
                fprintf(stderr, "%s: Invalid character is included in a tag '%s'. Tags can include alphabets, numbers, '_', '-', and '.'\n", PACKAGE_NAME, tags[i]);
            } else if (result == RESERVED_WORD_ERROR){
                fprintf(stderr, "%s: '%s' is a reserved word.\n", PACKAGE_NAME, tags[i]);
            }
            ret = INVALID_TAG_ERROR;
            goto cleanup;
        }
    }
    if (tags[ntags] != NULL){
        fprintf(stderr, "%s: Unknown Error: tags[%d] is not NULL\n", PACKAGE_NAME, ntags);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    result = file_init(dir, note_stock, list);
    if (result != 0){
        if (result == IO_ERROR){
            ret = IO_ERROR;
        } else{
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    result = get_datetime(clock, '\0', &datetime);
    if (result == MALLOC_ERROR){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    result = duplication_filter(&nkeys, keys);
    if (result != 0){
        fprintf(stderr, "%s: Unknown error by dulication_filter()\n", PACKAGE_NAME);
        ret = UNKNOWN_ERROR;
        goto cleanup;
    }

    result = duplication_filter(&ntags, tags);
    if (result != 0){
        fprintf(stderr, "%s: Unknown error by dulication_filter()\n", PACKAGE_NAME);
        ret = UNKNOWN_ERROR;
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
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
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
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
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

        // fprintf(stdout, "%s: New note was created: '%s'\n", PACKAGE_NAME, nexists[i]);
        printf("%s: New note was created: '%s'\n", PACKAGE_NAME, nexists[i]);

        XFREE(file);
        XFREE(path);
    }

    if (nexist_count == 1){
        printf("%s: %d key was created\n"  , PACKAGE_NAME, nexist_count);
    } else{
        printf("%s: %d keys were created\n", PACKAGE_NAME, nexist_count);
    }
    goto cleanup;


cleanup:
    if (xfclose(&fp)){
        if (ret == 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
            ret = IO_ERROR;
        }
    };
    free(datetime);
    free(path);
    free(file);
    free(exists);
    free(nexists);

    return ret;
}


