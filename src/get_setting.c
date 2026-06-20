
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "strutils.h"


// return INPUT_ERROR if s is empty
// return MALLOC_ERROR if malloc failed
// return 0 otherwise
int separate_words(char* s, int* n, char*** output){
    int i;
    int j;
    int prev_space;
    int curr_space;

    if (is_white_space(s)){
        return -1;
    }

    i = 0;
    *n = 0;
    prev_space = true;
    curr_space = true;
    #ifdef DEBUG
    printf("Word Counter\n");
    #endif
    while(s[i] != '\0'){
        if (s[i] != ' '){
            #ifdef DEBUG
            printf("%c is not a space", s[i]);
            #endif
            curr_space = false;
            if (prev_space){
                #ifdef DEBUG
                printf(" ... increment word counter\n");
                #endif
                *n = *n + 1;
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        } else {
            curr_space = true;
        }
        prev_space = curr_space;
        i = i + 1;
    }

    *output = malloc((*n) * sizeof(char*));
    if (*output == NULL){
        perror("malloc");
        return MALLOC_ERROR;
    }

    i = 0;
    j = 0;
    prev_space = true;
    curr_space = true;
    #ifdef DEBUG
    printf("Word Division\n");
    #endif
    while(s[i] != '\0'){
        if (s[i] != ' '){
            #ifdef DEBUG
            printf("%c is not a space", s[i]);
            #endif
            curr_space = false;
            if (prev_space){
                #ifdef DEBUG
                #endif
                (*output)[j] = &s[i];
                #ifdef DEBUG
                printf(" ... new word defined: %s\n", (*output)[j]);
                #endif
                j = j + 1;
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        } else {
            s[i] = '\0';
            curr_space = true;
        }
        prev_space = curr_space;
        i = i + 1;
    }

    #ifdef DEBUG
    printf("Number of Words: %d\n", *n);
    for (i = 0; i < *n; i = i + 1){
        printf("%s\n", (*output)[i]);
    }
    #endif

    return 0;
}


