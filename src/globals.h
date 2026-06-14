
#define FLAG_LEN        60                                  // Length of Keywords
#define DATETIME_LEN    20                                  // Length of datetime part of filename
#define EXT_LEN         8                                   // Length of Extention
#define FILE_LEN       (FLAG_LEN+2+DATETIME_LEN+1+EXT_LEN)  // Length of *.$EXT
#define LIST_LEN         8                                  // Length of $LISTNAME
#define HOME_LEN       512                                  // Length of $HOME/
#define DIR_LEN        (HOME_LEN+32)                        // Length of $HOME/$DIR/
#define SUBDIR_LEN     (DIR_LEN+8)                          // Length of $HOME/$DIR/$SUBDIR/
#define RC_LEN         (HOME_LEN+32)                        // Length of $HOME/$RCNAME
#define LIST_APATH_LEN (DIR_LEN+1+LIST_LEN)                 // Length of $HOME/$DIR/$LIST
#define FILE_APATH_LEN (SUBDIR_LEN+1+FILE_LEN)              // Length of $HOME/$DIR/$SUBDIR/*.txt
#define LS_LINE_LEN    128                                  // Length of the 1st line shown when ls run

#define PROGRAM  "sclipple"
#define DIR      ".sclipple"
#define EXT      "txt"
#define SUBDIR   "notes"
#define LISTNAME "list.csv"
#define RCNAME   ".sclipplerc"

// #define MAX_NUM_NOTE   2048                     // Maximum number of Notes

