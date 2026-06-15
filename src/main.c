
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "globals.h"
#include "help.h"
#include "get_setting.h"
#include "git_run.h"
#include "names.h"
#include "strutils.h"
#include "memo_add.h"
#include "memo_edit.h"
#include "memo_rm.h"
#include "memo_mv.h"
#include "memo_ls.h"
#include "memo_search.h"
#include "memo_show.h"

int main(int argc, char** argv){
    const char* rc_keys[] = {"editor", "extension"};
    char*       rc_vals[] = {EDITOR  , EXT};
    char** editor_commands;
    char*  home;
    char   dir[DIR_LEN];
    char   subdir[SUBDIR_LEN];
    char   rc[RC_LEN];
    char   list[LIST_APATH_LEN];
    char   ext[EXT_LEN];
    char   editor[EDITOR_LEN];
    int    nwords;
    int    result;
    int    i;
    time_t now;
    struct tm* lt;
    struct stat st;


    if (get_env("HOME", &home) == -1){
        return 1;
    }
    #ifdef DEBUG
    printf("$HOME = %s\n", home);
    #endif

    snprintf(rc, sizeof(rc), "%s/%s", home, RCNAME);
    if (stat(rc, &st) == 0){
        if (read_rc(rc, sizeof(rc_keys)/sizeof(rc_keys[0]), rc_keys, rc_vals) < 0){
            fprintf(stderr, "%s: Failed to read %s\n", PROGRAM, rc);
            return 1;
        }
    }

    snprintf(dir   , sizeof(dir)   , "%s/%s", home, DIR);
    snprintf(subdir, sizeof(subdir), "%s/%s", dir , SUBDIR);
    snprintf(list  , sizeof(list)  , "%s/%s", dir , LISTNAME);
    snprintf(editor, sizeof(editor), "%s"   , rc_vals[0]);
    snprintf(ext   , sizeof(ext)   , "%s"   , rc_vals[1]);
    #ifdef DEBUG
    printf("dir   = %s\n", dir);
    printf("subdir= %s\n", subdir);
    printf("rc    = %s\n", rc);
    printf("list  = %s\n", list);
    printf("ext   = %s\n", ext);
    #endif


    if (argc == 1){
        show_help();
        return 0;
    } else if (strcmp(argv[1], "git") == 0){
        git_run(dir, &argv[1]);
        return 0;
    }

    now = time(NULL);
    lt  = localtime(&now);

    if (strcmp(argv[1], "add") == 0){
        if (argc == 2){
            show_help();
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                result = add(list, dir, subdir, argv[i], ext, lt);
                if (result == -1){
                    fprintf(stderr, "%s: Failed to make new note: %s. %s exist.\n", PROGRAM, argv[i], argv[i]);
                } else if (result == -2){
                    fprintf(stderr, "%s IO Error\n", PROGRAM);
                }
            }
        }

    } else if (strcmp(argv[1], "rm") == 0){
        if (argc == 2){
            show_help();
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                result = rm(list, argv[i]);
                if (result < 0){
                    return 1;
                }
            }
        }

    } else if (strcmp(argv[1], "mv") == 0){
        if (argc != 4){
            show_help();
            return 0;
        } else{
            result = mv(list, argv[2], argv[3]);
            if (result < 0){
                return 1;
            }
        }

    } else if (strcmp(argv[1], "ls") == 0){
        result = ls(list, argc-2, &argv[2]);
        if (result < 0){
            return 1;
        }
    } else if (strcmp(argv[1], "search") == 0){
        if (argc == 2){
            show_help();
            return 0;
        } else{
            result = search(list, argv[2], argc-3, &argv[3]);
            if (result < 0){
                return 1;
            }
        }
    } else if (strcmp(argv[1], "show") == 0){
        result = show(list, argc-2, &argv[2]);
        if (result < 0){
            return 1;
        }
    } else {
        result = separate_words(editor, &nwords, &editor_commands);
        if (result == -1){
            fprintf(stderr, "%s Error: Specified editor command is empty\n", PROGRAM);
            return 1;
        } else if (result == -2){
            return 1;
        }
        result = memo_edit(list, subdir, editor_commands[0], nwords-1, &editor_commands[1], argc-1, &argv[1]);
        if (result == -1){
            return 1;
        }
    }

    return 0;
}

