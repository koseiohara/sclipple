

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


// return 1 if line is null or white space
// return 0 if line is not a white space
int is_white_space(const char* line){
    int i;

    i = 0;
    while(line[i] != '\0'){
        if (isspace((unsigned char)line[i]) == 0){
            return 0;
        }
        i = i + 1;
    }
    return 1;
}


// return -1 if '=' is not found
// return 0 otherwise
int line_to_dict(char* line, char* key, char* value){
    char* pt;

    // replace \n with \0
    line[strcspn(line, "\n")] = '\0';

    pt = strchr(line, '=');
    if (pt == NULL){
        return -1;
    }
    *pt   = '\0';
    key   = line;
    value = pt + 1;

    key   = trim(key);
    value = trim(value);

    return 0;
}


// return -1 when io error
int read_rc(const char* rc, const int n, const char** key, char** value){
    FILE*  fp;
    char*  line;
    char*  decomm;
    char   in_key[RC_KEY_LEN];
    char   in_value[RC_VALUE_LEN];
    int    i;
    size_t size;

    fp = fopen(rc, "r");
    if (fp == NULL){
        perror(rc);
        return -1;
    }

    line = NULL;
    size = 0;
    while(getline(&line, &size, fp) != -1){
        // delete comment
        decomm = strtok(line, "#");

        if (line_to_dict(decomm, in_key, in_value) < 0){
            continue;
        }

        for (i = 0; i < n; i = i + 1){
            if (strcmp(in_key, key[i]) == 0){
                snprintf(value[i], RC_VALUE_LEN, "%s", in_key);
            }
        }
    }
    fclose(fp);
    free(line);
    return 0;
}




