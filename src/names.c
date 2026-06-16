
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "globals.h"
#include "strutils.h"
#include "datetime.h"

int get_env(const char* env, char** output){
    *output = getenv(env);

    #ifdef DEBUG
    printf("<DEBUG> Expand %s: %s\n", env, *output);
    #endif
    if (*output == NULL){
        perror(env);
        return -1;
    }
    return 0;
}


int check_flag_length(char* flag){
    if (strlen(flag) > FLAG_LEN-1){
        return -1;
    }
    return 0;
}


// return -1 for invalid length
// return -2 for invalid character
// return  0 for valid ext
int ext_validation(const char* ext){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (ext == NULL){
        return -1;
    }

    len = strlen(ext);

    if (len >= EXT_LEN){
        return -1;
    }

    for (i = 0; i < len; i = i + 1){
        c = ext[i];
        if (isalnum(c) || c == '_' || c == '-' || c == '.'){
            continue;
        }
        return -2;
    }

    return 0;
}


// return -1 for invalid length
// return -2 for invalid character
// return  0 for valid flag
int flag_validation(const char* flag){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (flag == NULL){
        return -1;
    }

    len = strlen(flag);

    if (len >= FLAG_LEN || len == 0){
        return -1;
    }

    if (is_white_space(flag)){
        return -1;
    }

    // check banned character
    if (strcmp(flag, ".") == 0 || strcmp(flag, "..") == 0) {
        return -2;
    }

    for (i = 0; i < len; i = i + 1){
        c = flag[i];
        if (isalnum(c) || c == '_' || c == '-'){
            continue;
        }
        return -2;
    }

    if (strcmp(flag, "git") == 0){
        return -3;
    }

    if (strcmp(flag, "help") == 0){
        return -3;
    }

    if (strcmp(flag, "add") == 0){
        return -3;
    }

    if (strcmp(flag, "rm") == 0){
        return -3;
    }

    if (strcmp(flag, "mv") == 0){
        return -3;
    }

    if (strcmp(flag, "ls") == 0){
        return -3;
    }

    if (strcmp(flag, "search") == 0){
        return -3;
    }

    if (strcmp(flag, "show") == 0){
        return -3;
    }

    if (strcmp(flag, "show") == 0){
        return -3;
    }

    return 0;
}


void get_filename(const char* flag, char* datetime, char* ext, size_t output_len, char* output){
    snprintf(output, output_len, "%s--%s.%s", flag, datetime, ext);
}


int mv_filename(char* old_file, const char* new_flag, size_t output_len, char* output){
    char  tmp_old_file[FILE_APATH_LEN];
    char* cp;
    char* prefix;
    char* last;
    char* fname;

    // strcpy(tmp_old_file, old_file);
    snprintf(tmp_old_file, sizeof(tmp_old_file), "%s", old_file);
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
        snprintf(output, output_len, "%s%s%s", prefix, new_flag, last);
        return 0;
    } else{
        return -1;
    }
}


// return 1 if path exist
// return 0 if path does not exist
// return -1 if error other than ENOENT
int path_status(const char* path, struct stat* st){
    if (stat(path, st) == 0){
        return 1;
    }

    if (errno == ENOENT){
        return 0;
    }

    perror(path);
    return -1;
}


