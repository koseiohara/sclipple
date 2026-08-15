
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "globals.h"
#include "parse_options.h"
#include "help.h"
#include "git_run.h"
#include "file_systems.h"
#include "strutils.h"
#include "get_rc.h"
#include "memo_add.h"
#include "memo_rm.h"
#include "memo_mv.h"
#include "memo_ls.h"
#include "memo_search.h"
#include "memo_show.h"
#include "memo_tag.h"
#include "memo_edit.h"

int main(int argc, char** argv){
    Config  config = {0};
    RcEntry entry[N_ENTRY];
    char*   home;
    char*   subdir = NULL;
    char*   rc     = NULL;
    char*   list   = NULL;
    int     result;
    int     result_opt;
    int     ret;
    int     git_pos;

    time_t now;
    struct tm* lt;
    struct stat st;

    int    has_help    = false;
    int    has_version = false;
    int    nonoptsc;
    int    ntags;
    char** nonopts = NULL;
    char** tags    = NULL;


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
            if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
                ret = BUG_STOP;
            } else{
                ret = ERROR_STOP;
            }
            goto cleanup;
        }
    }

    nonopts = malloc(argc * sizeof(char*));
    tags    = malloc(argc * sizeof(char*));
    if (nonopts == NULL || tags == NULL){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = ERROR_STOP;
        goto cleanup;
    }
    result_opt = parse_opts(argc, argv,
                            &has_help, &has_version, &git_pos,
                            &nonoptsc, nonopts,
                            &ntags, tags,
                            &config
                           );

    if (result_opt != 0 && result_opt != GIT_FOUND){
        ret = ERROR_STOP;
        goto cleanup;
    }

    // printf("DEBUG: %s\n", config.dir);
    // printf("DEBUG: %s\n", config.ext);
    // printf("DEBUG: %s\n", config.editor);

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

    if (has_version == true){
        printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
        ret = 0;
        goto cleanup;
    }

    if (has_help == true){
        show_help_all(config.dir, subdir, list, rc);
        ret = STOP;
        goto cleanup;
    }

    // if (result_opt == INVALID_OPTION){
    //     // getopt will return error messages
    //     ret = ERROR_STOP;
    //     goto cleanup;
    // }

    if (result_opt == GIT_FOUND){
        result = git_run(config.dir, &argv[git_pos]);
        if (argc == 2){
            ret = NEGATIVE_STOP;
        } else if (result == 0){
            ret = STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

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

        result = add(list, config.dir, subdir, nonoptsc-1, &nonopts[1], ntags, tags, config.ext, lt);

        if (result == 0){
            ret = STOP;
        } else if (result == KEY_DUPLICATE){
            ret = NEGATIVE_STOP;
        } else if (result == INPUT_ERROR || result == UNKNOWN_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "rm") == 0){
        if (has_help == true){
            show_help_rm();
            if (has_help == true){
                ret = STOP;
            } else{
                ret = NEGATIVE_STOP;
            }
            goto cleanup;
        }

        result = rm(list, config.dir, TRASHDIR, TRASHFILE, nonoptsc-1, &nonopts[1], ntags, tags);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
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
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
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

        result = ls(list, nonoptsc-1, &nonopts[1], ntags, tags);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
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

        result = search(list, nonopts[1], nonoptsc-2, &nonopts[2], ntags, tags);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "show") == 0){
        if (has_help == true){
            show_help_show();
            ret = STOP;
            goto cleanup;
        }

        result = show(list, nonoptsc-1, &nonopts[1], ntags, tags);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "tag") == 0){
        if (has_help == true || nonoptsc <= 1 || ntags <= 0){
            show_help_tag();
            ret = STOP;
            goto cleanup;
        }

        result = tag(list, "tag", nonoptsc-1, &nonopts[1], ntags, tags);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
            ret = BUG_STOP;
        } else{
            ret = ERROR_STOP;
        }
        goto cleanup;
    }

    if (strcmp(nonopts[0], "untag") == 0){
        if (has_help == true || nonoptsc <= 1 || ntags <= 0){
            show_help_untag();
            ret = STOP;
            goto cleanup;
        }

        result = tag(list, "utag", nonoptsc-1, &nonopts[1], ntags, tags);
        if (result == 0){
            ret = STOP;
        } else if (result == KEY_NOT_FOUND){
            ret = NEGATIVE_STOP;
        } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
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

    result = memo_edit(list, subdir, config.editor, nonoptsc, nonopts, ntags, tags);
    if (result == 0){
        ret = STOP;
    } else if (result == KEY_NOT_FOUND){
        ret = NEGATIVE_STOP;
    } else if (result == UNKNOWN_ERROR || result == INPUT_ERROR){
        ret = BUG_STOP;
    } else{
        ret = ERROR_STOP;
    }
    goto cleanup;


cleanup:
    free_config(&config);
    free(subdir);
    free(rc);
    free(list);
    free(nonopts);
    free(tags);

    return ret;
}

