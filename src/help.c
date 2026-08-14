

#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "help.h"

static void print_separator(void){
    printf("\n");
}

void show_help_add(char* subdir, char* list){
    printf("ADD\n");
    printf("  Usage:\n");
    printf("    %s add KEY [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
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
    printf("  Tags:\n");
    printf("    Use -t TAG or --tag TAG to assign a tag when the notes are created.\n");
    printf("    Repeat the option to assign more than one tag. Duplicate tags are\n");
    printf("    ignored.\n");
    printf("\n");
    printf("  TAG rules:\n");
    printf("    - TAG may contain letters, digits, '_', '-' and '.'.\n");
    printf("    - TAG must not begin with '-'.\n");
    printf("\n");
    printf("  Files:\n");
    printf("    - Notes are stored under %s.\n", subdir);
    printf("    - The note index is stored at %s.\n", list);
    printf("    - The created filename has the form KEY.EXT.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s add <KEY>\n", PACKAGE_NAME);
    printf("    %s add <KEY1> <KEY2>\n", PACKAGE_NAME);
    printf("    %s add <KEY> --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s add <KEY1> <KEY2> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
}

void show_help_rm(void){
    printf("RM\n");
    printf("  Usage:\n");
    printf("    %s rm [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Remove notes selected by KEY or tag. The note file is deleted and the\n");
    printf("    matching KEY entry is removed from the index.\n");
    printf("\n");
    printf("  Tag selection:\n");
    printf("    Use -t TAG or --tag TAG to select notes having that tag. Repeat the\n");
    printf("    option to specify more than one tag. If both KEY and TAG are specified,\n");
    printf("    notes matching any KEY or any specified TAG are removed.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s rm <KEY>\n", PACKAGE_NAME);
    printf("    %s rm <KEY1> <KEY2>\n", PACKAGE_NAME);
    printf("    %s rm --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s rm <KEY> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
}

void show_help_mv(void){
    printf("MV\n");
    printf("  Usage:\n");
    printf("    %s mv OLD_KEY NEW_KEY\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Rename a note keyword. The index entry is updated and the note file is\n");
    printf("    renamed so that its filename begins with NEW_KEY.\n");
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
    printf("    %s ls [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    List notes. Without KEY or TAG, all notes are listed. KEY and TAG\n");
    printf("    arguments can be used to select notes.\n");
    printf("\n");
    printf("  Tag selection:\n");
    printf("    Use -t TAG or --tag TAG to list notes having that tag. Repeat the option\n");
    printf("    to specify more than one tag. If both KEY and TAG are specified, the\n");
    printf("    result is the union of notes matching any KEY or any specified TAG.\n");
    printf("\n");
    printf("  Output:\n");
    printf("    For each note, this command prints the keyword, creation timestamp,\n");
    printf("    file path, tags, and the first non-empty line of the note. Long first\n");
    printf("    lines are shortened.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s ls\n", PACKAGE_NAME);
    printf("    %s ls <KEY1> <KEY2>\n", PACKAGE_NAME);
    printf("    %s ls --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s ls <KEY> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
}

void show_help_search(void){
    printf("SEARCH\n");
    printf("  Usage:\n");
    printf("    %s search PATTERN [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Search note contents using a POSIX extended regular expression. The\n");
    printf("    search is case-insensitive. Without KEY or TAG, all notes are searched.\n");
    printf("    KEY and TAG arguments can be used to select notes before searching.\n");
    printf("\n");
    printf("  Tag selection:\n");
    printf("    Use -t TAG or --tag TAG to search notes having that tag. Repeat the\n");
    printf("    option to specify more than one tag. If both KEY and TAG are specified,\n");
    printf("    the selected notes match any KEY or any specified TAG.\n");
    printf("\n");
    printf("  Output:\n");
    printf("    Matching notes are printed with their keyword. Each matching line is\n");
    printf("    printed with its line number. When stdout is a terminal, keywords, line\n");
    printf("    numbers, and matches are colorized.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s search <PATTERN>\n", PACKAGE_NAME);
    printf("    %s search '<PATTERN1>|<PATTERN2>' <KEY>\n", PACKAGE_NAME);
    printf("    %s search <PATTERN> --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s search <PATTERN> <KEY> -t <TAG>\n", PACKAGE_NAME);
}

void show_help_show(void){
    printf("SHOW\n");
    printf("  Usage:\n");
    printf("    %s show [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Print full note contents to stdout. Without KEY or TAG, all notes are\n");
    printf("    shown. KEY and TAG arguments can be used to select notes.\n");
    printf("\n");
    printf("  Tag selection:\n");
    printf("    Use -t TAG or --tag TAG to show notes having that tag. Repeat the option\n");
    printf("    to specify more than one tag. If both KEY and TAG are specified, the\n");
    printf("    result is the union of notes matching any KEY or any specified TAG.\n");
    printf("\n");
    printf("  Output:\n");
    printf("    Each note begins with a [KEY] header. When stdout is a terminal, the\n");
    printf("    header is colorized.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s show\n", PACKAGE_NAME);
    printf("    %s show <KEY1> <KEY2>\n", PACKAGE_NAME);
    printf("    %s show --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s show <KEY> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
}

void show_help_tag(void){
    printf("TAG\n");
    printf("  Usage:\n");
    printf("    %s tag KEY [KEY ...] --tag TAG [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Add one or more tags to one or more existing notes. Duplicate KEY and\n");
    printf("    TAG arguments are ignored. If some KEY values do not exist, an error is\n");
    printf("    reported for them and the existing notes are still processed.\n");
    printf("\n");
    printf("  TAG rules:\n");
    printf("    - TAG may contain letters, digits, '_', '-' and '.'.\n");
    printf("    - TAG must not begin with '-'.\n");
    printf("    - Adding a tag that a note already has leaves that tag unchanged.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s tag <KEY> --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s tag <KEY1> <KEY2> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
}

void show_help_untag(void){
    printf("UNTAG\n");
    printf("  Usage:\n");
    printf("    %s untag KEY [KEY ...] --tag TAG [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Remove one or more tags from one or more existing notes. Duplicate KEY\n");
    printf("    and TAG arguments are ignored. If some KEY values do not exist, an error\n");
    printf("    is reported for them and the existing notes are still processed.\n");
    printf("\n");
    printf("  TAG rules:\n");
    printf("    - TAG may contain letters, digits, '_', '-' and '.'.\n");
    printf("    - TAG must not begin with '-'.\n");
    printf("    - Removing a tag that a note does not have leaves the note unchanged.\n");
    printf("\n");
    printf("  Examples:\n");
    printf("    %s untag <KEY> --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s untag <KEY1> <KEY2> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
}

void show_help_edit(char* rc){
    printf("EDIT\n");
    printf("  Usage:\n");
    printf("    %s KEY [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("  Description:\n");
    printf("    Edit one or more existing notes. Any first argument that is not a built-in\n");
    printf("    command is treated as a note KEY and opens selected notes in the\n");
    printf("    configured editor. Multiple KEY arguments can be specified.\n");
    printf("\n");
    printf("  Tag selection:\n");
    printf("    Use -t TAG or --tag TAG to additionally select notes having that tag.\n");
    printf("    Repeat the option to specify more than one tag. If both KEY and TAG are\n");
    printf("    specified, the selected notes match any KEY or any specified TAG.\n");
    printf("\n");
    printf("  Editor:\n");
    printf("    The default editor command is 'vim -p'. It can be changed with the\n");
    printf("    'editor' setting in %s.\n", rc);
    printf("\n");
    printf("  Examples:\n");
    printf("    %s <KEY>\n", PACKAGE_NAME);
    printf("    %s <KEY1> <KEY2>\n", PACKAGE_NAME);
    printf("    %s <KEY> --tag <TAG>\n", PACKAGE_NAME);
    printf("    %s <KEY1> -t <TAG1> -t <TAG2>\n", PACKAGE_NAME);
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
    printf("    directory = ~/notes\n");
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
    printf("  %s add KEY [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s rm [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s mv OLD_KEY NEW_KEY\n", PACKAGE_NAME);
    printf("  %s ls [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s search PATTERN [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s show [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s tag KEY [KEY ...] --tag TAG [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s untag KEY [KEY ...] --tag TAG [--tag TAG]...\n", PACKAGE_NAME);
    printf("  %s git GIT_ARGUMENTS...\n", PACKAGE_NAME);
    printf("  %s KEY [KEY ...] [--tag TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("TAG OPTION\n");
    printf("  -t TAG, --tag TAG\n");
    printf("    Repeat this option to specify multiple tags. With add, the tags are\n");
    printf("    assigned to newly created notes. With rm, ls, search, show, and edit,\n");
    printf("    tags select notes. The tag and untag commands use it to specify tags to\n");
    printf("    add or remove.\n");
    printf("\n");
    printf("  Selection semantics:\n");
    printf("    When a command supports selection by both KEY and TAG, notes matching\n");
    printf("    any KEY or any specified TAG are selected. Duplicate matches are merged.\n");
    printf("\n");
    printf("  TAG rules:\n");
    printf("    - TAG may contain letters, digits, '_', '-' and '.'.\n");
    printf("    - TAG must not begin with '-'.\n");
    printf("\n");
    printf("STORAGE\n");
    printf("  Directory: %s\n", dir);
    printf("  Notes:     %s\n", subdir);
    printf("  Index:     %s\n", list);
    printf("  Config:    %s\n", rc);
    printf("\n");
    printf("COMMAND HELP\n");
    printf("  Use '%s COMMAND --help' to show only one help section.\n", PACKAGE_NAME);
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
    show_help_tag();
    print_separator();
    show_help_untag();
    print_separator();
    show_help_edit(rc);
    print_separator();
    show_help_git();
    print_separator();
    show_help_config(rc);
}

