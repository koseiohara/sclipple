

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "strutils.h"


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


// return true  if line is null or white space
// return false if line is not a white space
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


int duplication_filter(int* nvals, char** vals){
    int i;
    int j;
    int n;
    int found;

    if (nvals == NULL || *nvals < 0 || vals == NULL){
        return INPUT_ERROR;
    } else if (*nvals <= 1){
        if (*nvals == 1 && vals[0] == NULL){
            *nvals = 0;
        }
        return 0;
    }

    n = *nvals;
    *nvals = 0;
    for (i = 0; i < n; i = i + 1){
        if (vals[i] == NULL){
            continue;
        }
        found = false;
        for (j = 0; j < *nvals; j = j + 1){
            if (strcmp(vals[i], vals[j]) == 0){
                found = true;
                break;
            }
        }

        if (found == false){
            vals[*nvals] = vals[i];
            *nvals = *nvals + 1;
        }
    }
    vals[*nvals] = NULL;

    return 0;
}


int arr2line(const int narr, char* const* arr, const char delim, const char left, const char right, char** line){
    char*   p;
    size_t* lens = NULL;
    int     left_len;
    int     right_len;
    int     i;
    int     ret;
    size_t  len;

    if (narr == 0){
        *line = malloc(sizeof(char));
        if (*line == NULL){
            ret = MALLOC_ERROR;
            goto cleanup;
        }
        **line = '\0';
        ret    = 0;
        goto cleanup;
    } else if (narr < 0){
        ret = INPUT_ERROR;
        goto cleanup;
    }

    lens = malloc((size_t)narr * sizeof(size_t));
    if (lens == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    if (left == '\0'){
        left_len = 0;
    } else{
        left_len = 1;
    }

    if (right == '\0'){
        right_len = 0;
    } else{
        right_len = 1;
    }

    len = 0;
    for (i = 0; i < narr; i = i + 1){
        lens[i] = strlen(arr[i]);
        len = len + lens[i];
    }
    len = len + narr * (2 + left_len + right_len);      // for a comma, space, left, and right

    *line = malloc(len * sizeof(char));
    if (*line == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    p = *line;
    for (i = 0; i < narr; i = i + 1){
        if (left != '\0'){
            *p = left;
            p = p + 1;
        }

        memcpy(p, arr[i], lens[i]);
        p  = p + lens[i];

        if (right != '\0'){
            *p = right;
            p = p + 1;
        }

        if (i != narr-1){
            *p = delim;
            p  = p + 1;
            *p = ' ';
            p  = p + 1;
        } else{
            *p = '\0';
        }
    }

    ret = 0;
    goto cleanup;


cleanup:
    free(lens);
    return ret;
}



