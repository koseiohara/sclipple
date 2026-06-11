
#ifdef DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>

#define true  1
#define false 0

void separate_words(char* editor, int* n, char*** output){
    int i;
    int j;
    int prev_space;
    int curr_space;
    
    i = 0;
    *n = 0;
    prev_space = true;
    curr_space = true;
    #ifdef DEBUG
    printf("Word Counter\n");
    #endif
    while(editor[i] != '\0'){
        if (editor[i] != ' '){
            #ifdef DEBUG
            printf("%c is not a space", editor[i]);
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

    i = 0;
    j = 0;
    prev_space = true;
    curr_space = true;
    #ifdef DEBUG
    printf("Word Division\n");
    #endif
    while(editor[i] != '\0'){
        if (editor[i] != ' '){
            #ifdef DEBUG
            printf("%c is not a space", editor[i]);
            #endif
            curr_space = false;
            if (prev_space){
                #ifdef DEBUG
                printf(" ... increment word counter\n");
                #endif
                (*output)[j] = &editor[i];
                j = j + 1;
            }
            #ifdef DEBUG
            printf("\n");
            #endif
        } else {
            editor[i] = '\0';
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
}


