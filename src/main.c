
// #include <stdlib.h>
#include <strings.h>

#include "help.h"
#include "get_setting.h"
#include "git_run.h"

int main(int argc, char** argv){
    char dir[] = "/home/kosei/CLib/build/note-taker";
    char editor[] = "vim -p";
    char** editor_commands;
    int  nwords;

    if (argc == 1){
        show_help();
    } else if (strcmp(argv[1], "git") == 0){
        git_run(dir, &argv[1]);
        return 0;
    }

    if (strcmp(argv[1], "add") == 0){
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

