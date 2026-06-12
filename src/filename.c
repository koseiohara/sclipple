
#include <stdio.h>
#include <stdlib.h>


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

void get_filename(const char* flag, char* output){
    sprintf(output, "%s.txt", flag);
}


