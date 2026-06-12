
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "datetime.h"

int get_env(const char* env, char** output){
    *output = getenv(env);

    #ifdef DEBUG
    printf("Expand %s: %s\n", env, *output);
    #endif
    if (*output == NULL){
        perror(env);
        return -1;
    }
    return 0;
}

void get_filename(const char* flag, char* datetime, char* ext, char* output){
    sprintf(output, "%s--%s.%s", flag, datetime, ext);
}


void mv_filename(char* output){
}


