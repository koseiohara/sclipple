

#include <string.h>
#include <ctype.h>


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


// return -2 if key is empty
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




