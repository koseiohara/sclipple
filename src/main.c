
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>

#include "globals.h"
#include "help.h"
#include "get_setting.h"
#include "git_run.h"
#include "names.h"
#include "memo_add.h"
#include "memo_edit.h"
#include "memo_rm.h"

int main(int argc, char** argv){
    char editor[] = "nvim -p";
    char** editor_commands;
    char*  home;
    char   dir[DIR_LEN];
    char   subdir[SUBDIR_LEN];
    char   rc[RC_LEN];
    char   list[LIST_APATH_LEN];
    char   ext[EXT_LEN];
    int    nwords;
    int    stat;
    int    i;
    time_t now;
    struct tm* lt;

    if (get_env("HOME", &home) == -1){
        return 1;
    }
    #ifdef DEBUG
    printf("$HOME = %s\n", home);
    #endif

    snprintf(dir   , sizeof(dir)   , "%s/%s", home, DIR);
    snprintf(subdir, sizeof(subdir), "%s/%s", dir , SUBDIR);
    snprintf(rc    , sizeof(rc)    , "%s/%s", home, RCNAME);
    snprintf(list  , sizeof(list)  , "%s/%s", dir , LISTNAME);
    strcpy(ext, EXT);
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
                stat = add(list, dir, subdir, argv[i], ext, lt);
                if (stat == -1){
                    fprintf(stderr, "Failed to make new note: %s. %s exist.\n", argv[i], argv[i]);
                }
            }
        }

    } else if (strcmp(argv[1], "rm") == 0){
        if (argc == 2){
            show_help();
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                stat = rm(list, argv[i]);
            }
        }

    } else if (strcmp(argv[1], "mv") == 0){
    } else if (strcmp(argv[1], "grep") == 0){
    } else if (strcmp(argv[1], "list") == 0){
    } else if (strcmp(argv[1], "show") == 0){
    } else {
        separate_words(editor, &nwords, &editor_commands);
        stat = memo_edit(list, subdir, editor_commands[0], nwords-1, &editor_commands[1], argc-1, &argv[1]);
        if (stat == -1){
            return 1;
        }
    }

    return 0;
}

