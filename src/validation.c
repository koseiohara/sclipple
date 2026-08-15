

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "globals.h"
#include "strutils.h"
#include "validation.h"


// return CHARACTER_NOT_ALLOWED_ERROR for invalid character
// return INPUT_ERROR if an argument is invalid
// return  0 for valid ext
int ext_validation(const char* ext){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (ext == NULL){
        return INPUT_ERROR;
    }

    len = strlen(ext);

    if (is_white_space(ext) == true){
        return INPUT_ERROR;
    }

    if (ext[0] == '.'){
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    for (i = 0; i < len; i = i + 1){
        c = ext[i];
        if (isalnum(c) || c == '_' || c == '-' || c == '.'){
            continue;
        }
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    return 0;
}


// return INPUT_ERROR if input is empty or not allocated
// return CHARACTER_NOT_ALLOWED_ERROR if key include invalid character
// return RESERVED_WORD_ERROR if input word is a reserved word
// return  0 for valid key
int key_validation(const char* key){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (key == NULL){
        return INPUT_ERROR;
    }

    len = strlen(key);

    if (is_white_space(key) == true){
        return INPUT_ERROR;
    }

    // check banned character
    if (strcmp(key, ".") == 0 || strcmp(key, "..") == 0) {
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    if (key[0] == '-'){
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    for (i = 0; i < len; i = i + 1){
        c = key[i];
        if (isalnum(c) || c == '_' || c == '-'){
            continue;
        }
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    if (strcmp(key, "git") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "add") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "tag") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "untag") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "rm") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "mv") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "ls") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "search") == 0){
        return RESERVED_WORD_ERROR;
    }

    if (strcmp(key, "show") == 0){
        return RESERVED_WORD_ERROR;
    }

    return 0;
}


// return INPUT_ERROR if input is empty or not allocated
// return CHARACTER_NOT_ALLOWED_ERROR if tag include invalid character
// return RESERVED_WORD_ERROR if input word is a reserved word
// return  0 for valid tag
int tag_validation(const char* tag){
    unsigned char c;
    size_t i;
    size_t len;

    // check length
    if (tag == NULL){
        return INPUT_ERROR;
    }

    len = strlen(tag);

    if (is_white_space(tag) == true){
        return INPUT_ERROR;
    }

    if (tag[0] == '-'){
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    for (i = 0; i < len; i = i + 1){
        c = tag[i];
        if (isalnum(c) || c == '_' || c == '-' || c == '.'){
            continue;
        }
        return CHARACTER_NOT_ALLOWED_ERROR;
    }

    // if (strcmp(tag, "git") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "add") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "tag") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "untag") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "rm") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "mv") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "ls") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "search") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    // if (strcmp(tag, "show") == 0){
    //     return RESERVED_WORD_ERROR;
    // }

    return 0;
}


