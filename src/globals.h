
#include "config.h"


#define LS_LINE_LEN 100             // Length of the 1st line shown when ls run

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "sclipple"
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unkown"
#endif

#define DIR        ".sclipple"
// #define EXT        "txt"
#define SUBDIR     "notes"
#define LISTNAME   ".list.csv"
#define RCNAME     ".sclipplerc"
// #define EDITOR     "nvim -p"
#define RC_COMMENT '#'

#define STOP 0
#define NEGATIVE_STOP 1
#define ERROR_STOP 2
#define BUG_STOP 3

#define IO_ERROR 1
#define MALLOC_ERROR 2
#define LIST_FORMAT_ERROR 3
#define FILE_FORMAT_ERROR 4
#define INPUT_ERROR 5
#define RENAME_ERROR 6
#define UNLINK_ERROR 7
#define RESERVED_WORD_ERROR 8
#define CHARACTER_NOT_ALLOWED_ERROR 9
#define ACCESS_FAILED_ERROR 10
#define INVALID_KEY_ERROR 11
#define REGEX_ERROR 12
#define WORDEXP_ERROR 13
#define RC_ERROR 14
#define FORK_ERROR 15
#define EXECVP_ERROR 16
#define CHILD_ERROR 17
#define WAIT_ERROR 18
#define UNKNOWN_ERROR 100

#define false -1
#define true  -2
#define KEY_NOT_FOUND -3
#define KEY_DUPLICATE -4
#define LIST_WHITE_SPACE -5
#define PATH_EXIST -6
#define PATH_NOT_EXIST -7
#define END_OF_FILE -8

