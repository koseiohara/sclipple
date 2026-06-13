
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


void mv_filename(const char* old_file, const char* new_flag, size_t output_len, char* output){
    char* cp;
    char* last;

    cp = old_file;
    while ((cp = strstr(cp, "--")) != NULL){
        last = cp;
        cp = cp + 2;
    }
    #ifdef DEBUG
    printf("<DEBUG> File name after '--': %s\n", last);
    #endif

    snprintf(output, output_len, "%s%s", new_flag, last);
}


