
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "globals.h"
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
    if (strlen(flag) > FLAG_LEN){
        return -1;
    }
    return 0;
}


void get_filename(const char* flag, char* datetime, char* ext, size_t output_len, char* output){
    snprintf(output, output_len, "%s--%s.%s", flag, datetime, ext);
}


void mv_filename(char* old_file, const char* new_flag, size_t output_len, char* output){
    char  tmp_old_file[FILE_APATH_LEN];
    char* cp;
    char* prefix;
    char* last;
    char* fname;

    strcpy(tmp_old_file, old_file);
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

    snprintf(output, output_len, "%s%s%s", prefix, new_flag, last);
}


