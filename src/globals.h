
#define FLAG_LEN         60                                     // Length of Keywords
#define DATETIME_LEN     20                                     // Length of datetime part of filename
#define EXT_LEN          32                                     // Length of Extention
#define FILE_LEN        (FLAG_LEN+2+DATETIME_LEN+1+EXT_LEN)     // Length of *.$EXT
#define LIST_LEN         16                                     // Length of $LISTNAME
#define HOME_LEN        512                                     // Length of $HOME/
#define DIR_LEN         (HOME_LEN+32)                           // Length of $HOME/$DIR/
#define SUBDIR_LEN      (DIR_LEN+8)                             // Length of $HOME/$DIR/$SUBDIR/
#define RC_LEN          (HOME_LEN+32)                           // Length of $HOME/$RCNAME
#define LIST_APATH_LEN  (DIR_LEN+1+LIST_LEN)                    // Length of $HOME/$DIR/$LIST
#define FILE_APATH_LEN  (SUBDIR_LEN+1+FILE_LEN)                 // Length of $HOME/$DIR/$SUBDIR/*.txt
#define LS_LINE_LEN     100                                     // Length of the 1st line shown when ls run
// #define LINE_LEN        32768                                   // Maximum length of line for search
#define RC_KEY_LEN       32                                     // Length of left hand side in rc file
#define RC_VALUE_LEN    128                                     // Length of right hand side in rc file
#define EDITOR_LEN       32                                     // Length of editor command ($EDITOR)

#define PROGRAM    "sclipple"
#define DIR        ".sclipple"
// #define EXT        "txt"
#define SUBDIR     "notes"
#define LISTNAME   ".list.csv"
#define RCNAME     ".sclipplerc"
// #define EDITOR     "nvim -p"
#define RC_COMMENT '#'

// #define MAX_NUM_NOTE   2048                     // Maximum number of Notes

#define true  1
#define false 0

