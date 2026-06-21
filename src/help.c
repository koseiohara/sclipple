#include <stdio.h>
#include <string.h>

#include "globals.h"

static void print_separator(void){
    printf("\n");
}

void show_help_add(char* subdir, char* list){
    printf("ADD\n");
    printf("  Usage:\n");
    printf("    %s add KEY [KEY ...]\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Create one or more new notes. Each KEY becomes the note keyword.\n");
    printf("    When the storage directory or list file does not exist, this command\n");
    printf("    initializes them automatically.\n");
    printf("\n");
    printf("  KEY rules:\n");
    printf("    - KEY may contain ASCII letters, digits, '_' and '-'.\n");
    printf("    - '.' and '..' are not valid KEY values.\n");
    printf("    - KEY must not already exist.\n");
    printf("\n");
    printf("  Files:\n");
    printf("    - Notes are stored under %s.\n", subdir);
    printf("    - The note index is stored at %s.\n", list);
    printf("    - The created filename has the form KEY--YYYY-MM-DD-hh-mm-ss.EXT.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s add <KEY>\n", PACKAGE_NAME);
    printf("    %s add <KEY1> <KEY2>\n", PACKAGE_NAME);
}

void show_help_rm(void){
    printf("RM\n");
    printf("  Usage:\n");
    printf("    %s rm KEY [KEY ...]\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Remove one or more notes. The note file is deleted and the matching\n");
    printf("    KEY entry is removed from the index.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s rm <KEY>\n", PACKAGE_NAME);
    printf("    %s rm <KEY1> <KEY2>\n", PACKAGE_NAME);
}

void show_help_mv(void){
    printf("MV\n");
    printf("  Usage:\n");
    printf("    %s mv OLD_KEY NEW_KEY\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Rename a note keyword. The index entry is updated and the note file is\n");
    printf("    renamed so that its filename begins with NEW_KEY. The timestamp part is\n");
    printf("    preserved.\n");
    printf("\n");
    printf("  NEW_KEY rules:\n");
    printf("    - NEW_KEY must follow the same validation rules as add KEY.\n");
    printf("    - NEW_KEY must not already exist.\n");
    printf("\n");
    printf("  Example:\n");
    printf("    %s mv <OLD_KEY> <NEW_KEY>\n", PACKAGE_NAME);
}

void show_help_ls(void){
    printf("LS\n");
    printf("  Usage:\n");
    printf("    %s ls [KEY ...]\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    List notes. Without KEY, all notes are listed. With one or more KEY\n");
    printf("    arguments, only those notes are listed in the requested order.\n");
    printf("\n");
    printf("  Output:\n");
    printf("    For each note, this command prints the keyword, creation timestamp, and\n");
    printf("    the first non-empty line of the note. Long first lines are shortened.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s ls\n", PACKAGE_NAME);
    printf("    %s ls <KEY1> <KEY2>\n", PACKAGE_NAME);
}

void show_help_search(void){
    printf("SEARCH\n");
    printf("  Usage:\n");
    printf("    %s search PATTERN [KEY ...]\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Search note contents using a POSIX extended regular expression. The\n");
    printf("    search is case-insensitive. Without KEY, all notes are searched. With\n");
    printf("    one or more KEY arguments, only those notes are searched.\n");
    printf("\n");
    printf("  Output:\n");
    printf("    Matching notes are printed with their keyword. Each matching line is\n");
    printf("    printed with its line number. When stdout is a terminal, keywords, line\n");
    printf("    numbers, and matches are colorized.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s search <PATTERN>\n", PACKAGE_NAME);
    printf("    %s search '<PATTERN1>|<PATTERN2>' <KEY>\n", PACKAGE_NAME);
}

void show_help_show(void){
    printf("SHOW\n");
    printf("  Usage:\n");
    printf("    %s show [KEY ...]\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Print full note contents to stdout. Without KEY, all notes are shown.\n");
    printf("    With one or more KEY arguments, only those notes are shown in the\n");
    printf("    requested order.\n");
    printf("\n");
    printf("  Output:\n");
    printf("    Each note begins with a [KEY] header. When stdout is a terminal, the\n");
    printf("    header is colorized.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s show\n", PACKAGE_NAME);
    printf("    %s show <KEY1> <KEY2>\n", PACKAGE_NAME);
}

void show_help_edit(char* rc){
    printf("EDIT\n");
    printf("  Usage:\n");
    printf("    %s KEY [KEY ...]\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Edit one or more existing notes. Any first argument that is not a built-in\n");
    printf("    command is treated as a note KEY and opens the note in the configured\n");
    printf("    editor. Multiple KEY arguments open multiple notes.\n");
    printf("\n");
    printf("  Editor:\n");
    printf("    The default editor command is 'vim -p'. It can be changed with the\n");
    printf("    'editor' setting in %s.\n", rc);
    printf("\n");
    printf("  Examples:\n");
    printf("    %s <KEY>\n", PACKAGE_NAME);
    printf("    %s <KEY1> <KEY2>\n", PACKAGE_NAME);
}

void show_help_git(void){
    printf("GIT\n");
    printf("  Usage:\n");
    printf("    %s git GIT_ARGUMENTS...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Run git inside the configured note storage directory.\n");
    printf("    Arguments are passed directly to git, so ordinary git subcommands can\n");
    printf("    be used.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s git status\n", PACKAGE_NAME);
    printf("    %s git init\n", PACKAGE_NAME);
    printf("    %s git add .\n", PACKAGE_NAME);
    printf("    %s git commit -m 'update notes'\n", PACKAGE_NAME);
}

void show_help_config(char* rc){
    printf("CONFIGURATION\n");
    printf("  File:\n");
    printf("    %s\n", rc);
    printf("\n");
    printf("  Syntax:\n");
    printf("    key = value\n");
    printf("\n");
    printf("  Supported keys:\n");
    printf("    editor     Editor command used by the edit command. Default: vim -p\n");
    printf("    extension  File extension for newly created notes. Default: txt\n");
    printf("    directory  Directory used to store sclipple data. Default: $HOME/%s\n", DIR);
    printf("\n");
    printf("  Notes:\n");
    printf("    - Lines beginning with '#' are treated as comments.\n");
    printf("    - Surrounding single or double quotes around values are removed.\n");
    printf("\n");
    printf("  Example:\n");
    printf("    editor = 'nvim -p'\n");
    printf("    extension = md\n");
    printf("    extension = ~/notes\n");
}

void show_help_all(char* dir, char* subdir, char* list, char* rc){
    printf("\n");
    printf("NAME\n");
    printf("  %s - small command-line memo manager using keyword-based notes\n", PACKAGE_NAME);
    printf("\n");
    printf("VERSION\n");
    printf("  %s\n", PACKAGE_VERSION);
    printf("\n");
    printf("SYNOPSIS\n");
    printf("  %s add KEY [KEY ...]\n", PACKAGE_NAME);
    printf("  %s rm KEY [KEY ...]\n", PACKAGE_NAME);
    printf("  %s mv OLD_KEY NEW_KEY\n", PACKAGE_NAME);
    printf("  %s ls [KEY ...]\n", PACKAGE_NAME);
    printf("  %s search PATTERN [KEY ...]\n", PACKAGE_NAME);
    printf("  %s show [KEY ...]\n", PACKAGE_NAME);
    printf("  %s git GIT_ARGUMENTS...\n", PACKAGE_NAME);
    printf("  %s KEY [KEY ...]\n", PACKAGE_NAME);
    printf("  %s help [COMMAND|config|all]\n", PACKAGE_NAME);
    printf("\n");
    printf("STORAGE\n");
    printf("  Directory: %s\n", dir);
    printf("  Notes:     %s\n", subdir);
    printf("  Index:     %s\n", list);
    printf("  Config:    %s\n", rc);
    printf("\n");
    printf("COMMAND HELP\n");
    printf("  Use '%s help COMMAND' to show only one help section.\n", PACKAGE_NAME);
    printf("\n");
    show_help_add(subdir, list);
    print_separator();
    show_help_rm();
    print_separator();
    show_help_mv();
    print_separator();
    show_help_ls();
    print_separator();
    show_help_search();
    print_separator();
    show_help_show();
    print_separator();
    show_help_edit(rc);
    print_separator();
    show_help_git();
    print_separator();
    show_help_config(rc);
}

void show_help_command(const char* command, char* dir, char* subdir, char* list, char* rc){
    if (command == NULL || strcmp(command, "all") == 0){
        show_help_all(dir, subdir, list, rc);
    } else if (strcmp(command, "add") == 0){
        show_help_add(subdir, list);
    } else if (strcmp(command, "rm") == 0){
        show_help_rm();
    } else if (strcmp(command, "mv") == 0){
        show_help_mv();
    } else if (strcmp(command, "ls") == 0){
        show_help_ls();
    } else if (strcmp(command, "search") == 0){
        show_help_search();
    } else if (strcmp(command, "show") == 0){
        show_help_show();
    } else if (strcmp(command, "edit") == 0){
        show_help_edit(rc);
    } else if (strcmp(command, "git") == 0){
        show_help_git();
    } else if (strcmp(command, "config") == 0 || strcmp(command, "rc") == 0){
        show_help_config(rc);
    } else{
        fprintf(stderr, "%s Error: Unknown help topic: %s\n", PACKAGE_NAME, command);
        fprintf(stderr, "Available topics: add, rm, mv, ls, search, show, edit, git, config, all\n");
    }
}

void show_help(char* dir, char* subdir, char* list, char* rc){
    show_help_all(dir, subdir, list, rc);
}
