
#ifndef _GLOBALS_H
#define _GLOBALS_H


#include "config.h"


#define LS_LINE_LEN 100             // Length of the 1st line shown when ls run

#ifndef PACKAGE_NAME
#define PACKAGE_NAME "sclipple"
#endif

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "unkown"
#endif

#define DIR        ".sclipple"
#define SUBDIR     "notes"
#define TRASHDIR   "trash"
#define TRASHFILE  "trash"
#define LISTNAME   ".list.csv"
#define RCNAME     ".sclipplerc"
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
#define INVALID_TAG_ERROR 12
#define REGEX_ERROR 13
#define WORDEXP_ERROR 14
#define RC_ERROR 15
#define FORK_ERROR 16
#define EXECVP_ERROR 17
#define CHILD_ERROR 18
#define WAIT_ERROR 19
#define IS_NOT_DIRECTORY_ERROR 20
#define MKDIR_ERROR 21
#define EDITOR_ERROR 22
#define UNKNOWN_ERROR 100


#define false 64
#define true  65
#define KEY_NOT_FOUND 66
#define KEY_DUPLICATE 67
#define LIST_WHITE_SPACE 68
#define PATH_EXIST 69
#define PATH_NOT_EXIST 70
#define END_OF_FILE 71
#define RESULT_EMPTY 72
#define IS_DIRECTORY 73

#endif

