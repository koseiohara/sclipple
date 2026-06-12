
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#include "globals.h"
#include "help.h"
#include "get_setting.h"
#include "git_run.h"
#include "filename.h"
#include "memo_add.h"
#include "memo_edit.h"

int main(int argc, char** argv){
    char editor[] = "nvim -p";
    char** editor_commands;
    char*  home;
    char   dir[DIR_LEN];
    char   subdir[SUBDIR_LEN];
    char   rc[RC_LEN];
    int    nwords;
    int    stat;
    int    i;

    if (get_env("HOME", &home) == -1){
        return 1;
    }
    #ifdef DEBUG
    printf("$HOME = %s\n", home);
    #endif

    sprintf(dir   , "%s/%s", home, DIR);
    sprintf(subdir, "%s/%s", dir , SUBDIR);
    sprintf(rc    , "%s/%s", home, RCNAME);
    #ifdef DEBUG
    printf("dir = %s\n", dir);
    printf("rc  = %s\n", rc);
    #endif


    if (argc == 1){
        show_help();
        return 0;
    } else if (strcmp(argv[1], "git") == 0){
        git_run(dir, &argv[1]);
        return 0;
    }

    if (strcmp(argv[1], "add") == 0){
        if (argc == 2){
            show_help();
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                stat = add(LISTNAME, dir, subdir, argv[i]);
                if (stat == -1){
                    fprintf(stderr, "Failed to make new note: %s. %s is exist.\n", argv[i], argv[i]);
                }
            }
        }

    } else if (strcmp(argv[1], "rm") == 0){
    } else if (strcmp(argv[1], "mv") == 0){
    } else if (strcmp(argv[1], "grep") == 0){
    } else if (strcmp(argv[1], "list") == 0){
    } else if (strcmp(argv[1], "show") == 0){
    } else {
        separate_words(editor, &nwords, &editor_commands);
// int memo_edit(const char* dir, char* editor, const int editor_options_num, char* const* editor_options, const int file_num, char* const* file){
        stat = memo_edit(subdir, editor_commands[0], nwords, &editor_commands[1], argc, &argv[1]);
        if (stat == -1){
            return 1;
        }
    }

    return 0;
}

