
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <wordexp.h>

#include "globals.h"
#include "strutils.h"
#include "ptrutils.h"
#include "datetime.h"
#include "file_systems.h"


// return INPUT_ERROR if invalid env
// return 0 otherwise
int get_env(const char* env, char** output){
    *output = getenv(env);

    #ifdef DEBUG
    printf("<DEBUG> Expand %s: %s\n", env, *output);
    #endif
    if (*output == NULL){
        return INPUT_ERROR;
    }
    return 0;
}


// return INPUT_ERROR if arguments are invalid
// return WORDEXP_ERROR if failed to parse
// return MALLOC_ERROR if strdup failed
// return RC_ERROR if specified is not the absolute path
// return 0 otherwise
int parse_directory(const char* input_dir, char** output_dir){
    wordexp_t  we;
    char* tmp;
    char* tmp_trim;
    int result;
    size_t len;

    if (output_dir == NULL){
        return INPUT_ERROR;
    }

    *output_dir = NULL;
    if (input_dir == NULL || input_dir[0] == '\0'){
        return INPUT_ERROR;
    }

    *output_dir = NULL;

    result = wordexp(input_dir, &we, WRDE_NOCMD | WRDE_UNDEF);
    if (result != 0){
        if (result == WRDE_NOSPACE){
            wordfree(&we);
        }
        return WORDEXP_ERROR;
    }
    
    if (we.we_wordc != 1 || we.we_wordv == NULL || we.we_wordv[0] == NULL){
        wordfree(&we);
        return WORDEXP_ERROR;
    }

    tmp = strdup(we.we_wordv[0]);
    wordfree(&we);
    if (tmp == NULL){
        return MALLOC_ERROR;
    }

    tmp_trim = trim(tmp);

    if (tmp_trim[0] != '/'){
        XFREE(tmp);
        return RC_ERROR;
    }

    *output_dir = strdup(tmp_trim);
    XFREE(tmp);
    if (*output_dir == NULL){
        return MALLOC_ERROR;
    }

    len = strlen(*output_dir);
    while (len > 1 && (*output_dir)[len - 1] == '/') {
        (*output_dir)[len - 1] = '\0';
        len = len - 1;
    }

    return 0;
}


// return MALLOC_ERROR if MALLOC failed
// return 0 otherwise
int get_filename(const char* key, char* ext, char** output){
    int result;

    result = asprintf(output,  "%s.%s", key, ext);
    if (result < 0){
        return MALLOC_ERROR;
    }
    return 0;
}


// return FILE_FORMAT_ERROR if old file name does not incude "--"
// return MALLOC_ERROR if asprintf failed
// return 0 otherwise
int mv_filename(char* old_file, const char* new_key, char** output){
    char* tmp_old_file = NULL;
    char* cp;
    char* prefix;
    char* last;
    char* fname;
    int   result;

    // strcpy(tmp_old_file, old_file);
    // snprintf(tmp_old_file, sizeof(tmp_old_file), "%s", old_file);
    tmp_old_file = strdup(old_file);
    if (tmp_old_file == NULL){
        return MALLOC_ERROR;
    }
    cp    = tmp_old_file;
    fname = tmp_old_file;

    #ifdef DEBUG
    printf("<DEBUG> mv_filename: %s\n", cp);
    #endif

    while ((cp = strrchr(cp, '/')) != NULL){
        cp = cp + 1;
        fname = cp;

        #ifdef DEBUG
        printf("<DEBUG> mv_filename: %s\n", cp);
        #endif
    }

    #ifdef DEBUG
    printf("<DEBUG> mv_filename: Last / was found\n");
    #endif

    *fname = '\0';
    prefix = tmp_old_file;
    cp     = fname + 1;

    #ifdef DEBUG
    printf("<DEBUG> mv_filename: prefix is %s\n", prefix);
    #endif

    // last = NULL;
    // while ((cp = strstr(cp, ".")) != NULL){
    //     last = cp;
    //     cp = cp + 2;
    //     #ifdef DEBUG
    //     printf("<DEBUG> mv_filename: %s", cp);
    //     #endif
    // }
    // #ifdef DEBUG
    // printf("<DEBUG> File name after '--': %s\n", last);
    // #endif

    last = strchr(cp, '.');
    if (last != NULL){
        result = asprintf(output, "%s%s%s", prefix, new_key, last);
        XFREE(tmp_old_file);     // tmp_old_file must not be freed before asprintf because prefix and last share the memory with tmp_old_file
        if (result < 0){
            return MALLOC_ERROR;
        } else{
            return 0;
        }
    } else{
        XFREE(tmp_old_file);
        return FILE_FORMAT_ERROR;
    }
}


// return PATH_EXIST if path exist
// return PATH_NOT_EXIST if path does not exist
// return ACCESS_FAILED_ERROR if error other than ENOENT
int path_status(const char* path, struct stat* st){
    if (stat(path, st) == 0){
        return PATH_EXIST;
    }

    if (errno == ENOENT){
        return PATH_NOT_EXIST;
    }

    return ACCESS_FAILED_ERROR;
}


