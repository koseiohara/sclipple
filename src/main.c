
// #include <stdlib.h>
#include <strings.h>
#include <string.h>

#include "globals.h"
#include "help.h"
#include "get_setting.h"
#include "git_run.h"
#include "memo_add.h"

int main(int argc, char** argv){
    char editor[] = "vim -p";
    char** editor_commands;
    int  nwords;
    int  i;

    if (argc == 1){
        show_help();
        return 0;
    } else if (strcmp(argv[1], "git") == 0){
        git_run(DIR, &argv[1]);
        return 0;
    }

    if (strcmp(argv[1], "add") == 0){
        if (argc == 2){
            show_help();
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                add(LISTNAME, DIR, SUBDIR, argv[i]);
            }
        }

    } else if (strcmp(argv[1], "rm") == 0){
    } else if (strcmp(argv[1], "mv") == 0){
    } else if (strcmp(argv[1], "grep") == 0){
    } else if (strcmp(argv[1], "list") == 0){
    } else if (strcmp(argv[1], "show") == 0){
    } else {
        separate_words(editor, &nwords, &editor_commands);
    }

    return 0;
}

