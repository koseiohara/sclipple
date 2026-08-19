

#include <stdio.h>

#include "globals.h"
#include "help.h"

static void show_common_selection_options(void){
    printf("  -t, --tag TAG          Select notes by tag; may be repeated.\n");
    printf("      --tag-match MODE   Match repeated tags using 'and' or 'or'.\n");
    printf("      --directory DIR    Use DIR as the storage directory.\n");
}

static void show_selection_rules(void){
    printf("Multiple TAG selectors use tag-match: 'or' matches any requested TAG,\n");
    printf("while 'and' matches all requested TAGs. If both KEY and TAG are specified,\n");
    printf("notes matching either selection are selected.\n");
}

void show_help_add(void){
    printf("Usage: %s add KEY [KEY ...] [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("Create one or more notes.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -t, --tag TAG          Add TAG to the new notes; may be repeated.\n");
    printf("      --directory DIR    Use DIR as the storage directory.\n");
    printf("      --extension EXT    Use EXT for newly created note files.\n");
    printf("  -h, --help             Show help.\n");
    printf("\n");
    printf("KEY may contain ASCII letters, digits, '_' and '-'.\n");
    printf("EXT may contain letters, digits, '.', '-' and '_', and must not begin\n");
    printf("with '.'.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s add todo\n", PACKAGE_NAME);
    printf("  %s add todo idea --tag work\n", PACKAGE_NAME);
    printf("  %s add todo --extension md\n", PACKAGE_NAME);
}

void show_help_rm(void){
    printf("Usage: %s rm [KEY ...] [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("Remove notes selected by KEY or tag.\n");
    printf("\n");
    printf("Options:\n");
    show_common_selection_options();
    printf("  -h, --help             Show help.\n");
    printf("\n");
    show_selection_rules();
}

void show_help_mv(void){
    printf("Usage: %s mv OLD_KEY NEW_KEY [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("Rename a note.\n");
    printf("\n");
    printf("Options:\n");
    printf("      --directory DIR    Use DIR as the storage directory.\n");
    printf("  -h, --help             Show help.\n");
}

void show_help_ls(void){
    printf("Usage: %s ls [KEY ...] [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("List notes. Without KEY or TAG, list all notes.\n");
    printf("\n");
    printf("Options:\n");
    show_common_selection_options();
    printf("  -s, --short            List keys and tags in the short format\n");
    printf("  -h, --help             Show help.\n");
    printf("\n");
    show_selection_rules();
}

void show_help_search(void){
    printf("Usage: %s search PATTERN [KEY ...] [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("Search note contents using a case-insensitive POSIX extended regular\n");
    printf("expression. Without KEY or TAG, search all notes.\n");
    printf("\n");
    printf("Options:\n");
    show_common_selection_options();
    printf("  -h, --help             Show help.\n");
    printf("\n");
    show_selection_rules();
}

void show_help_show(void){
    printf("Usage: %s show [KEY ...] [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("Print full note contents. Without KEY or TAG, show all notes.\n");
    printf("\n");
    printf("Options:\n");
    show_common_selection_options();
    printf("  -h, --help             Show help.\n");
    printf("\n");
    show_selection_rules();
}

void show_help_tag(void){
    printf("Usage: %s tag KEY [KEY ...] --tag TAG [--tag TAG]... [OPTIONS]\n",
           PACKAGE_NAME);
    printf("\n");
    printf("Add one or more tags to existing notes.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -t, --tag TAG          Tag to add; may be repeated.\n");
    printf("      --directory DIR    Use DIR as the storage directory.\n");
    printf("  -h, --help             Show help.\n");
    printf("\n");
    printf("TAG may contain letters, digits, '_', '-' and '.', and must not begin\n");
    printf("with '-'.\n");
}

void show_help_untag(void){
    printf("Usage: %s untag KEY [KEY ...] --tag TAG [--tag TAG]... [OPTIONS]\n",
           PACKAGE_NAME);
    printf("\n");
    printf("Remove one or more tags from existing notes.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -t, --tag TAG          Tag to remove; may be repeated.\n");
    printf("      --directory DIR    Use DIR as the storage directory.\n");
    printf("  -h, --help             Show help.\n");
    printf("\n");
    printf("TAG may contain letters, digits, '_', '-' and '.', and must not begin\n");
    printf("with '-'.\n");
}

void show_help_edit(void){
    printf("Usage: %s [KEY ...] [OPTIONS]\n", PACKAGE_NAME);
    printf("\n");
    printf("Open notes selected by KEY or tag in the configured editor. A first argument\n");
    printf("that is not a built-in command is treated as a note KEY. At least one KEY or\n");
    printf("TAG must be specified.\n");
    printf("\n");
    printf("Options:\n");
    show_common_selection_options();
    printf("      --editor COMMAND   Use COMMAND instead of the configured editor.\n");
    printf("  -h, --help             Show help.\n");
    printf("\n");
    show_selection_rules();
}

void show_help_git(void){
    printf("Usage: %s [--directory DIR] git GIT_ARGUMENTS...\n", PACKAGE_NAME);
    printf("\n");
    printf("Run git in the sclipple storage directory. Arguments after 'git' are passed\n");
    printf("directly to git.\n");
    printf("\n");
    printf("Options:\n");
    printf("      --directory DIR    Use DIR as the storage directory. This option must\n");
    printf("                         appear before 'git'.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s git status\n", PACKAGE_NAME);
    printf("  %s --directory /path/to/notes git log --oneline\n", PACKAGE_NAME);
}

void show_help_config(void){
    printf("Configuration file:\n");
    printf("  ~/%s\n", RCNAME);
    printf("\n");
    printf("Settings:\n");
    printf("  directory = PATH      Storage directory. Default: $HOME/%s\n", DIR);
    printf("  extension = EXT       Extension for newly created notes. Default: txt\n");
    printf("  editor = COMMAND      Editor used to open notes. Default: vim -p\n");
    printf("  tag-match = MODE      Match repeated tags with 'and' or 'or'. Default: or\n");
    printf("\n");
    printf("Command-line overrides:\n");
    printf("  --directory DIR       Override 'directory'. DIR must be an absolute path.\n");
    printf("  --extension EXT       Override 'extension'.\n");
    printf("  --editor COMMAND      Override 'editor'.\n");
    printf("  --tag-match MODE      Override 'tag-match'. MODE must be 'and' or 'or'.\n");
    printf("\n");
    printf("The configuration file uses 'key = value' syntax. Lines beginning with '#'\n");
    printf("are comments. Surrounding single or double quotes around values are removed.\n");
}

void show_help_all(void){
    printf("Usage: %s [OPTIONS] COMMAND [ARGS...]\n", PACKAGE_NAME);
    printf("       %s [OPTIONS] KEY [KEY ...]\n", PACKAGE_NAME);
    printf("       %s [OPTIONS] -t TAG [-t TAG]...\n", PACKAGE_NAME);
    printf("\n");
    printf("Small command-line memo manager using keyword-based notes.\n");
    printf("\n");
    printf("Commands:\n");
    printf("  add       Create one or more notes\n");
    printf("  rm        Remove notes\n");
    printf("  mv        Rename a note\n");
    printf("  ls        List notes\n");
    printf("  search    Search note contents\n");
    printf("  show      Print full note contents\n");
    printf("  tag       Add tags to notes\n");
    printf("  untag     Remove tags from notes\n");
    printf("  git       Run git in the storage directory\n");
    printf("\n");
    printf("KEYs or TAGs given without a command open the selected notes in the configured\n");
    printf("editor.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help             Show help\n");
    printf("  -v, --version          Show version\n");
    printf("  -s, --short            List keys and tags in the short format\n");
    printf("  -t, --tag TAG          Specify or select a tag; may be repeated\n");
    printf("      --tag-match MODE   Match repeated tags using 'and' or 'or'\n");
    printf("      --directory DIR    Override the storage directory\n");
    printf("      --extension EXT    Override the extension for newly created notes\n");
    printf("      --editor COMMAND   Override the editor command\n");
    printf("\n");
    printf("Run '%s COMMAND --help' for help on a command.\n", PACKAGE_NAME);
    printf("\n");
    show_help_config();
}


