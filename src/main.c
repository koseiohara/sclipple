
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <getopt.h>

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
    Config  config = {0};
    RcEntry entry[N_ENTRY];
    // char** editor_commands = NULL;
    char*  home;
    char*  subdir = NULL;
    char*  rc     = NULL;
    char*  list   = NULL;
    int    result;
    int    ret;

    time_t now;
    struct tm* lt;
    struct stat st;

    int    option;
    int    has_help = false;
    int    has_tag  = false;
    int    nonoptsc;
    int    tagc;
    char** nonopts = NULL;
    char** tags    = NULL;

    static const struct option opt_list[] = {
                                             {"help"   , no_argument      , NULL, 'h'},
                                             {"version", no_argument      , NULL, 'v'},
                                             {"tag"    , required_argument, NULL, 't'},
                                             {NULL     , 0                , NULL,  0 },
                                            };


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
            if (result == UNKNOWN_ERROR){
                ret = BUG_STOP;
            } else{
                ret = ERROR_STOP;
            }
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

    if (strcmp(argv[1], "git") == 0){
        result = git_run(config.dir, &argv[1]);
        if (argc == 2){
            ret = NEGATIVE_STOP;
        } else if (result == 0){
            ret = STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    nonoptsc = 0;
    tagc     = 0;
    nonopts  = malloc(argc * sizeof(char*));
    tags     = malloc(argc * sizeof(char*));
    while ((option = getopt_long(argc, argv, "-hvt:", opt_list, NULL)) != -1){
        switch (option){
            case 'h':
                has_help = true;
                break;
            case 'v':
                // has_version = true;
                printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
                ret = STOP;
                goto cleanup;
                // break;
            case 't':
                has_tag = true;
                tags[tagc] = optarg;
                tagc = tagc + 1;
                break;
            case 1:
                nonopts[nonoptsc] = optarg;
                nonoptsc = nonoptsc + 1;
                break;
            case '?':
                ret = ERROR_STOP;
                goto cleanup;
        }
    }
    nonopts[nonoptsc] = NULL;
    tags[tagc]        = NULL;

    if (nonoptsc == 0){
        show_help_all(config.dir, subdir, list, rc);
        ret = STOP;
        goto cleanup;
    }

    if (strcmp(nonopts[0], "add") == 0){
        if (has_help == true || nonoptsc == 1){
            show_help_add(subdir, list);
            if (has_help == true){
                ret = STOP;
            } else{
                ret = NEGATIVE_STOP;
            }
            goto cleanup;
        }

        now = time(NULL);
        lt  = localtime(&now);

        result = add(list, config.dir, subdir, nonoptsc-1, &nonopts[1], config.ext, lt);

        if (result == 0){
            ret = STOP;
        } else if (result == KEY_DUPLICATE){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "rm") == 0){
        if (has_help == true || nonoptsc == 1){
            show_help_rm();
            if (has_help == true){
                ret = STOP;
            } else{
                ret = NEGATIVE_STOP;
            }
            goto cleanup;
        }

        result = rm(list, nonoptsc-1, &nonopts[1]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "mv") == 0){
        if (has_help == true || nonoptsc != 3){
            show_help_mv();
            if (has_help == true){
                ret = STOP;
            } else{
                ret = NEGATIVE_STOP;
            }
            goto cleanup;
        }

        result = mv(list, nonopts[1], nonopts[2]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND || result == KEY_DUPLICATE){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "ls") == 0){
        if (has_help == true){
            show_help_ls();
            ret = STOP;
            goto cleanup;
        }

        result = ls(list, nonoptsc-1, &nonopts[1]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "search") == 0){
        if (has_help == true || nonoptsc == 1){
            show_help_search();
            if (has_help == true){
                ret = STOP;
            } else{
                ret = NEGATIVE_STOP;
            }
            goto cleanup;
        }

        result = search(list, nonopts[1], nonoptsc-2, &nonopts[2]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "show") == 0){
        if (has_help == true){
            show_help_show();
            if (nonoptsc == 1){
                ret = STOP;
            } else{
                ret = NEGATIVE_STOP;
            }
            goto cleanup;
        }

        result = show(list, nonoptsc-1, &nonopts[1]);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (has_help == true){
        show_help_all(config.dir, subdir, list, rc);
        ret = NEGATIVE_STOP;
        goto cleanup;
    }

    result = memo_edit(list, subdir, config.editor, nonoptsc, nonopts);
    if (result == 0){
        ret = STOP;
    } else if (result == KEY_NOT_FOUND){
        ret = NEGATIVE_STOP;
    } else if (result == UNKNOWN_ERROR){
        ret = BUG_STOP;
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
    free(nonopts);
    free(tags);

    return ret;
}

