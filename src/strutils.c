

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"


void leftrim(char** s){
    while (*s != NULL && **s != '\0'){
        if (isspace((unsigned char)**s) == 0){
            break;
        }
        *s = *s + 1;
    }
}


void rightrim(char* s){
    int    i;
    size_t size;

    size = strlen(s);
    for (i = (int)size-1; i >= 0; i = i - 1){
        if (isspace((unsigned char)s[i]) == 0){
            s[i+1] = '\0';
            return;
        }
    }
    s[0] = '\0';
}


char* trim(char* s){
    if (s == NULL){
        return NULL;
    }

    leftrim(&s);
    rightrim(s);

    return s;
}


// // return IO_ERROR if asprintf failed
// // return 0 otherwise
// int cat(char** out, char* a, char* b, char* delim){
//     int result;
//     // int len;

//     // *out = NULL;

//     // len = strlen(a) + strlen(b) + strlen(delim) + 1;        // the last +1 is for \0
//     // *out = malloc(len);

//     // if (*out == NULL){
//     //     perror("cat");
//     //     return MALLOC_ERROR;
//     // }

//     // snprintf(*out, len, "%s%s%s", a, delim, b);
//     result = asprintf(out, "%s%s%s", a, delim, b);
//     if (result == -1){
//         return IO_ERROR;
//     }
//     return 0;
// }


// return 1 if line is null or white space
// return 0 if line is not a white space
int is_white_space(const char* line){
    int i;

    i = 0;
    while(line[i] != '\0'){
        if (isspace((unsigned char)line[i]) == 0){
            return false;
        }
        i = i + 1;
    }
    return true;
}


// return INPUT_ERROR if '=' is not found or memory not allocated
// return 0 otherwise
int line_to_dict(char* line, char** key, char** value){
    char* pt;

    if (line == NULL){
        return INPUT_ERROR;
    }

    // replace \n with \0
    line[strcspn(line, "\n")] = '\0';

    pt = strchr(line, '=');
    if (pt == NULL){
        return INPUT_ERROR;
    }
    *pt    = '\0';
    *key   = line;
    *value = pt + 1;

    *key   = trim(*key);
    *value = trim(*value);

    return 0;
}


void delete_bracket(char** s, int n, const char* lbracket, const char* rbracket){
    int last;
    int i;
    int count;

    if (s == NULL || *s == NULL) {
        return;
    }

    last = (int)strlen(*s) - 1;
    if (last < 1){
        return;
    }

    while (true){
        count = 0;
        for (i = 0; i < n; i = i + 1){
            if ((*s)[0] == lbracket[i] && (*s)[last] == rbracket[i]){
                (*s)[last] = '\0';
                *s = *s + 1;
                last = last - 2;
                count = count + 1;

                if (last < 1){
                    return;
                }
            }
        }
        if (count == 0){
            return;
        }
    }
}



