
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
#include "datetime.h"


// return INPUT_ERROR if invalid env
// return 0 otherwise
int get_env(const char* env, char** output){
    *output = getenv(env);

    #ifdef DEBUG
    printf("<DEBUG> Expand %s: %s\n", env, *output);
    #endif
    if (*output == NULL){
        perror(env);
        return INPUT_ERROR;
    }
    return 0;
}


// return INPUT_ERROR for invalid character
// return  0 for valid ext
int ext_validation(const char* ext){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (ext == NULL){
        return INPUT_ERROR;
    }

    len = strlen(ext);

    for (i = 0; i < len; i = i + 1){
        c = ext[i];
        if (isalnum(c) || c == '_' || c == '-' || c == '.'){
            continue;
        }
        return INPUT_ERROR;
    }

    return 0;
}


// return INPUT_ERROR if input is empty or not allocated
// return CHARACTER_NOT_ALLOWED_ERROR if flag include invalid character
// return RESERVED_WORD_ERROR if input word is a reserved word
// return  0 for valid flag
int flag_validation(const char* flag){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (flag == NULL){
        return INPUT_ERROR;
    }

    len = strlen(flag);

    if (is_white_space(flag) == true){
        return INPUT_ERROR;
    }

    // check banned character
    if (strcmp(flag, ".") == 0 || strcmp(flag, "..") == 0) {
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    if (flag[0] == '-'){
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    for (i = 0; i < len; i = i + 1){
        c = flag[i];
        if (isalnum(c) || c == '_' || c == '-'){
            continue;
        }
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    if (strcmp(flag, "git") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "help") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "add") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "rm") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "mv") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "ls") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "search") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(flag, "show") == 0){
        return RESERVED_WORD_ERROR;
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
    int result;

    if (output_dir == NULL){
        return INPUT_ERROR;
    }

    if (input_dir == NULL || input_dir[0] == '\0'){
        return INPUT_ERROR;
    }

    *output_dir = NULL;

    result = wordexp(input_dir, &we, WRDE_NOCMD | WRDE_UNDEF);
    if (result != 0){
        return WORDEXP_ERROR;
    }
    
    if (we.we_wordc != 1 || we.we_wordv == NULL || we.we_wordv[0] == NULL){
        wordfree(&we);
        return WORDEXP_ERROR;
    }

    *output_dir = strdup(we.we_wordv[0]);
    wordfree(&we);
    if (*output_dir == NULL){
        perror("strdup");
        return MALLOC_ERROR;
    }

    *output_dir = trim(*output_dir);

    if (*(output_dir[0]) != '/'){
        free(*output_dir);
        *output_dir = NULL;
        return RC_ERROR;
    }

    return 0;
}


// return MALLOC_ERROR if MALLOC failed
// return 0 otherwise
int get_filename(const char* flag, char* datetime, char* ext, char** output){
    int result;

    result = asprintf(output,  "%s--%s.%s", flag, datetime, ext);
    if (result < 0){
        perror("asprintf");
        return MALLOC_ERROR;
    }
    return 0;
}


// return FILE_FORMAT_ERROR if old file name does not incude "--"
// return MALLOC_ERROR if asprintf failed
// return 0 otherwise
int mv_filename(char* old_file, const char* new_flag, char** output){
    char* tmp_old_file = NULL;
    char* cp;
    char* prefix;
    char* last;
    char* fname;
    int   result;

    // strcpy(tmp_old_file, old_file);
    // snprintf(tmp_old_file, sizeof(tmp_old_file), "%s", old_file);
    tmp_old_file = strdup(old_file);
    cp    = tmp_old_file;
    fname = tmp_old_file;

    #ifdef DEBUG
    printf("<DEBUG> mv_filename: %s\n", cp);
    #endif

    while ((cp = strstr(cp, "/")) != NULL){
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

    last = NULL;
    while ((cp = strstr(cp, "--")) != NULL){
        last = cp;
        cp = cp + 2;
        #ifdef DEBUG
        printf("<DEBUG> mv_filename: %s", cp);
        #endif
    }
    #ifdef DEBUG
    printf("<DEBUG> File name after '--': %s\n", last);
    #endif

    if (last != NULL){
        result = asprintf(output, "%s%s%s", prefix, new_flag, last);
        free(tmp_old_file);     // tmp_old_file must not be freed before asprintf because prefix and last share the memory with tmp_old_file
        if (result < 0){
            perror("asprintf");
            return MALLOC_ERROR;
        } else{
            return 0;
        }
    } else{
        free(tmp_old_file);
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

    perror(path);
    return ACCESS_FAILED_ERROR;
}


