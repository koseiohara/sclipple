
#include "config.h"

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
#include "get_rc.h"
#include "memo_add.h"
#include "memo_edit.h"
#include "memo_rm.h"
#include "memo_mv.h"
#include "memo_ls.h"
#include "memo_search.h"
#include "memo_show.h"

int main(int argc, char** argv){
    Config  config;
    RcEntry entry[N_ENTRY];
    char** editor_commands=NULL;
    char*  home;
    char*  dir;
    char*  subdir;
    char*  rc;
    char*  list;
    int    nwords;
    int    result;
    int    i;
    time_t now;
    struct tm* lt;
    struct stat st;


    if (get_env("HOME", &home) < 0){
        return 1;
    }
    #ifdef DEBUG
    printf("$HOME = %s\n", home);
    #endif

    result = asprintf(&rc, "%s/%s", home, RCNAME);
    if (result < 0){
        perror("asprintf");
        return 1;
    }
    init(&config, entry);
    if (path_status(rc, &st) == PATH_EXIST){
        result = read_rc(rc, entry, sizeof(entry) / sizeof(entry[0]));
        if (result < 0){
            if (result == INPUT_ERROR){
                free_config(&config);
                free(rc);
                return 1;
            } else if (result == IO_ERROR){
                free_config(&config);
                free(rc);
                return 1;
            }
        }
    }

    result = asprintf(&dir, "%s/%s", home, DIR);
    if (result < 0){
        perror("asprintf");
        free(rc);
        return 1;
    }
    result = asprintf(&subdir, "%s/%s", dir, SUBDIR);
    if (result < 0){
        perror("asprintf");
        free(rc);
        free(dir);
        return 1;
    }
    result = asprintf(&list, "%s/%s", dir, LISTNAME);
    if (result < 0){
        perror("asprintf");
        free(rc);
        free(dir);
        free(subdir);
        return 1;
    }
    #ifdef DEBUG
    printf("dir   = %s\n", dir);
    printf("subdir= %s\n", subdir);
    printf("rc    = %s\n", rc);
    printf("list  = %s\n", list);
    printf("ext   = %s\n", config.ext);
    #endif


    if (argc == 1){
        show_help_all();
        free_config(&config);
        free(rc);
        free(dir);
        free(subdir);
        free(list);
        return 0;
    } else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0){
        if (argc == 2){
            show_help_all();
        } else{
            for (i = 2; i < argc; i = i + 1){
                show_help_command(argv[i]);
                if (i + 1 < argc){
                    printf("\n");
                }
            }
        }
        free_config(&config);
        free(rc);
        free(dir);
        free(subdir);
        free(list);
        return 0;
    } else if (strcmp(argv[1], "git") == 0){
        result = git_run(dir, &argv[1]);
        free_config(&config);
        free(rc);
        free(dir);
        free(subdir);
        free(list);
        if (result < 0){
            return 1;
        } else{
            return 0;
        }
    }

    now = time(NULL);
    lt  = localtime(&now);

    if (strcmp(argv[1], "add") == 0){
        if (argc == 2){
            show_help_add();
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                result = add(list, dir, subdir, argv[i], config.ext, lt);
                if (result >= 0 || result == INVALID_KEY_ERROR){
                    continue;
                }
                if (result < 0){
                    free_config(&config);
                    free(rc);
                    free(dir);
                    free(subdir);
                    free(list);
                    return 1;
                }
            }
        }

    } else if (strcmp(argv[1], "rm") == 0){
        if (argc == 2){
            show_help_rm();
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 0;
        } else{
            for (i = 2; i < argc; i = i + 1){
                result = rm(list, argv[i]);
                if (result < 0 || result == KEY_NOT_FOUND){
                    free_config(&config);
                    free(rc);
                    free(dir);
                    free(subdir);
                    free(list);
                    return 1;
                }
            }
        }

    } else if (strcmp(argv[1], "mv") == 0){
        if (argc != 4){
            show_help_mv();
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 0;
        } else{
            result = mv(list, argv[2], argv[3]);
            if (result < 0 || result == KEY_NOT_FOUND || result == KEY_DUPLICATE){
                free_config(&config);
                free(rc);
                free(dir);
                free(subdir);
                free(list);
                return 1;
            }
        }

    } else if (strcmp(argv[1], "ls") == 0){
        result = ls(list, argc-2, &argv[2]);
        if (result < 0 || result == KEY_NOT_FOUND){
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 1;
        }
    } else if (strcmp(argv[1], "search") == 0){
        if (argc == 2){
            show_help_search();
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 0;
        } else{
            result = search(list, argv[2], argc-3, &argv[3]);
            if (result < 0 || result == KEY_NOT_FOUND){
                free_config(&config);
                free(rc);
                free(dir);
                free(subdir);
                free(list);
                return 1;
            }
        }
    } else if (strcmp(argv[1], "show") == 0){
        result = show(list, argc-2, &argv[2]);
        if (result < 0 || result == KEY_NOT_FOUND){
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 1;
        }
    } else {
        result = separate_words(config.editor, &nwords, &editor_commands);
        if (result < 0){
            free(editor_commands);
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 1;
        }
        result = memo_edit(list, subdir, editor_commands[0], nwords-1, &editor_commands[1], argc-1, &argv[1]);
        if (result < 0 || result == KEY_NOT_FOUND){
            free(editor_commands);
            free_config(&config);
            free(rc);
            free(dir);
            free(subdir);
            free(list);
            return 1;
        }
    }

    free(editor_commands);
    free_config(&config);
    free(rc);
    free(dir);
    free(subdir);
    free(list);
    return 0;
}

