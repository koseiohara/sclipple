
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "globals.h"
#include "help.h"
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
    // char** editor_commands = NULL;
    char*  home;
    char*  subdir = NULL;
    char*  rc     = NULL;
    char*  list   = NULL;
    int    result;
    int    ret;
    int    i;
    time_t now;
    struct tm* lt;
    struct stat st;


    if (get_env("HOME", &home) != 0){
        ret = ERROR_STOP;
        goto cleanup;
    }

    result = asprintf(&rc, "%s/%s", home, RCNAME);
    if (result < 0){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = ERROR_STOP;
        goto cleanup;
    }

    result = init(&config, entry, home);
    if (result != 0){
        ret = ERROR_STOP;
        goto cleanup;
    }

    if (path_status(rc, &st) == PATH_EXIST){
        result = read_rc(rc, entry, sizeof(entry) / sizeof(entry[0]));
        if (result != 0){
            ret = ERROR_STOP;
            goto cleanup;
        }
    }

    result = asprintf(&subdir, "%s/%s", config.dir, SUBDIR);
    if (result < 0){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = ERROR_STOP;
        goto cleanup;
    }
    result = asprintf(&list, "%s/%s", config.dir, LISTNAME);
    if (result < 0){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = ERROR_STOP;
        goto cleanup;
    }

    if (argc == 1){
        show_help_all(config.dir, subdir, list, rc);
        ret = STOP;
        goto cleanup;
    }

    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0){
        if (argc == 2){
            show_help_all(config.dir, subdir, list, rc);
        } else{
            for (i = 2; i < argc; i = i + 1){
                show_help_command(argv[i], config.dir, subdir, list, rc);
                if (i + 1 < argc){
                    printf("\n");
                }
            }
        }
        ret = STOP;
        goto cleanup;
    }

    if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0){
        printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
        ret = STOP;
        goto cleanup;
    }

    if (strcmp(argv[1], "git") == 0){
        result = git_run(config.dir, &argv[1]);
        if (argc == 2){
            ret = NEGATIVE_STOP;
        } else if (result == 0){
            ret = STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(argv[1], "add") == 0){
        if (argc == 2){
            show_help_add(subdir, list);
            ret = NEGATIVE_STOP;
            goto cleanup;
        }

        now = time(NULL);
        lt  = localtime(&now);

        result = add(list, config.dir, subdir, argc-2, &argv[2], config.ext, lt);

        if (result == 0){
            ret = STOP;
        } else if (result == KEY_DUPLICATE){
            ret = NEGATIVE_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(argv[1], "rm") == 0){
        if (argc == 2){
            show_help_rm();
            ret = NEGATIVE_STOP;
            goto cleanup;
        }

        result = rm(list, argc-2, &argv[2]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(argv[1], "mv") == 0){
        if (argc != 4){
            show_help_mv();
            ret = NEGATIVE_STOP;
            goto cleanup;
        }

        result = mv(list, argv[2], argv[3]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND || result == KEY_DUPLICATE){
            ret = NEGATIVE_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(argv[1], "ls") == 0){
        result = ls(list, argc-2, &argv[2]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(argv[1], "search") == 0){
        if (argc == 2){
            show_help_search();
            ret = NEGATIVE_STOP;
            goto cleanup;
        } else{
            result = search(list, argv[2], argc-3, &argv[3]);
            if (result == 0){
                ret = STOP;
            } else if (result == KEY_NOT_FOUND){
                ret = NEGATIVE_STOP;
            } else{
                ret = ERROR_STOP;
            }
            goto cleanup;
        }
    }

    if (strcmp(argv[1], "show") == 0){
        result = show(list, argc-2, &argv[2]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    result = memo_edit(list, subdir, config.editor, argc-1, &argv[1]);
    if (result == 0){
        ret = STOP;
    } else if (result == KEY_NOT_FOUND){
        ret = NEGATIVE_STOP;
    } else{
        ret = ERROR_STOP;
    }
    goto cleanup;


cleanup:
    free_config(&config);
    // free(editor_commands);
    free(subdir);
    free(rc);
    free(list);

    return ret;
}

