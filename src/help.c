#include <stdio.h>
#include <string.h>

#include "globals.h"

static void print_separator(void){
    printf("\n");
}

void show_help_add(void){
    printf("ADD\n");
    printf("  Usage:\n");
    printf("    %s add KEY [KEY ...]\n", PROGRAM);
    printf("\n");
    printf("  Description:\n");
    printf("    Create one or more new notes. Each KEY becomes the note keyword.\n");
    printf("    When the storage directory or list file does not exist, this command\n");
    printf("    initializes them automatically.\n");
    printf("\n");
    printf("  KEY rules:\n");
    printf("    - KEY must be shorter than %d characters.\n", FLAG_LEN);
    printf("    - KEY may contain ASCII letters, digits, '_' and '-'.\n");
    printf("    - '.' and '..' are not valid KEY values.\n");
    printf("    - KEY must not already exist.\n");
    printf("\n");
    printf("  Files:\n");
    printf("    - Notes are stored under $HOME/%s/%s.\n", DIR, SUBDIR);
    printf("    - The note index is stored at $HOME/%s/%s.\n", DIR, LISTNAME);
    printf("    - The created filename has the form KEY--YYYY-MM-DD-hh-mm-ss.EXT.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s add foo\n", PROGRAM);
    printf("    %s add bar baz\n", PROGRAM);
}

void show_help_rm(void){
    printf("RM\n");
    printf("  Usage:\n");
    printf("    %s rm KEY [KEY ...]\n", PROGRAM);
    printf("\n");
    printf("  Description:\n");
    printf("    Remove one or more notes. The note file is deleted and the matching\n");
    printf("    KEY entry is removed from the index.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s rm foo\n", PROGRAM);
    printf("    %s rm bar baz\n", PROGRAM);
}

void show_help_mv(void){
    printf("MV\n");
    printf("  Usage:\n");
    printf("    %s mv OLD_KEY NEW_KEY\n", PROGRAM);
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
    printf("    %s mv tmp permanent_note\n", PROGRAM);
}

void show_help_ls(void){
    printf("LS\n");
    printf("  Usage:\n");
    printf("    %s ls [KEY ...]\n", PROGRAM);
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
    printf("    %s ls\n", PROGRAM);
    printf("    %s ls idea todo\n", PROGRAM);
}

void show_help_search(void){
    printf("SEARCH\n");
    printf("  Usage:\n");
    printf("    %s search PATTERN [KEY ...]\n", PROGRAM);
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
    printf("    %s search wave\n", PROGRAM);
    printf("    %s search 'Sunny|Day' rainy\n", PROGRAM);
}

void show_help_show(void){
    printf("SHOW\n");
    printf("  Usage:\n");
    printf("    %s show [KEY ...]\n", PROGRAM);
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
    printf("    %s show\n", PROGRAM);
    printf("    %s show idea todo\n", PROGRAM);
}

void show_help_edit(void){
    printf("EDIT\n");
    printf("  Usage:\n");
    printf("    %s KEY [KEY ...]\n", PROGRAM);
    printf("\n");
    printf("  Description:\n");
    printf("    Edit one or more existing notes. Any first argument that is not a built-in\n");
    printf("    command is treated as a note KEY and opens the note in the configured\n");
    printf("    editor. Multiple KEY arguments open multiple notes.\n");
    printf("\n");
    printf("  Editor:\n");
    printf("    The default editor command is 'vim -p'. It can be changed with the\n");
    printf("    'editor' setting in $HOME/%s.\n", RCNAME);
    printf("\n");
    printf("  Examples:\n");
    printf("    %s idea\n", PROGRAM);
    printf("    %s idea todo\n", PROGRAM);
}

void show_help_git(void){
    printf("GIT\n");
    printf("  Usage:\n");
    printf("    %s git GIT_ARGUMENTS...\n", PROGRAM);
    printf("\n");
    printf("  Description:\n");
    printf("    Run git inside the note storage directory $HOME/%s. The word 'git' is\n", DIR);
    printf("    passed through as argv[0], so ordinary git subcommands can be used after\n");
    printf("    it.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s git status\n", PROGRAM);
    printf("    %s git init\n", PROGRAM);
    printf("    %s git add .\n", PROGRAM);
    printf("    %s git commit -m 'update notes'\n", PROGRAM);
}

void show_help_config(void){
    printf("CONFIGURATION\n");
    printf("  File:\n");
    printf("    $HOME/%s\n", RCNAME);
    printf("\n");
    printf("  Syntax:\n");
    printf("    key = value\n");
    printf("\n");
    printf("  Supported keys:\n");
    printf("    editor     Editor command used by the edit command. Default: vim -p\n");
    printf("    extension  File extension for newly created notes. Default: txt\n");
    printf("\n");
    printf("  Notes:\n");
    printf("    - Lines may include comments beginning with '#'.\n");
    printf("    - Surrounding single or double quotes around values are removed.\n");
    printf("\n");
    printf("  Example:\n");
    printf("    editor = 'nvim -p'\n");
    printf("    extension = md\n");
}

void show_help_all(void){
    printf("\n");
    printf("NAME\n");
    printf("  %s - small command-line memo manager using keyword-based notes\n", PROGRAM);
    printf("\n");
    printf("SYNOPSIS\n");
    printf("  %s add KEY [KEY ...]\n", PROGRAM);
    printf("  %s rm KEY [KEY ...]\n", PROGRAM);
    printf("  %s mv OLD_KEY NEW_KEY\n", PROGRAM);
    printf("  %s ls [KEY ...]\n", PROGRAM);
    printf("  %s search PATTERN [KEY ...]\n", PROGRAM);
    printf("  %s show [KEY ...]\n", PROGRAM);
    printf("  %s git GIT_ARGUMENTS...\n", PROGRAM);
    printf("  %s KEY [KEY ...]\n", PROGRAM);
    printf("  %s help [COMMAND|config|all]\n", PROGRAM);
    printf("\n");
    printf("STORAGE\n");
    printf("  Directory: $HOME/%s\n", DIR);
    printf("  Notes:     $HOME/%s/%s\n", DIR, SUBDIR);
    printf("  Index:     $HOME/%s/%s\n", DIR, LISTNAME);
    printf("  Config:    $HOME/%s\n", RCNAME);
    printf("\n");
    printf("COMMAND HELP\n");
    printf("  Use '%s help COMMAND' to show only one help section.\n", PROGRAM);
    printf("\n");
    show_help_add();
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
    show_help_edit();
    print_separator();
    show_help_git();
    print_separator();
    show_help_config();
}

void show_help_command(const char* command){
    if (command == NULL || strcmp(command, "all") == 0){
        show_help_all();
    } else if (strcmp(command, "add") == 0){
        show_help_add();
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
        show_help_edit();
    } else if (strcmp(command, "git") == 0){
        show_help_git();
    } else if (strcmp(command, "config") == 0 || strcmp(command, "rc") == 0){
        show_help_config();
    } else{
        fprintf(stderr, "%s Error: Unknown help topic: %s\n", PROGRAM, command);
        fprintf(stderr, "Available topics: add, rm, mv, ls, search, show, edit, git, config, all\n");
    }
}

void show_help(void){
    show_help_all();
}


