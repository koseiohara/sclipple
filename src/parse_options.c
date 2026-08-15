

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include "globals.h"
#include "ptrutils.h"
#include "file_systems.h"
#include "validation.h"
#include "parse_options.h"

enum {
    OPT_DIRECTORY = 256,
    OPT_EXTENSION,
    OPT_EDITOR,
};


// return MALLOC_ERROR if malloc failed
// FILE_FORMAT_ERROR if input directory does not start with /
// CHARACTER_NOT_ALLOWED_ERROR if invalid character is included in extension
// INVALID_OPTION if unknown option is obtained
// WORDEXP_ERROR if directory is invalid
// UNKNOWN_ERROR if a bug is found
// GIT_FOUND if the first non-option argument is git
// 0 otherwise
int parse_opts(int argc, char** argv, int* has_help, int* has_version, int* git_pos, int* nonoptsc, char** nonopts, int* ntags, char** tags, Config* config){
    int opt;
    int result;
    static const struct option opt_list[] = {
                                             {"help"     , no_argument      , NULL, 'h'},
                                             {"version"  , no_argument      , NULL, 'v'},
                                             {"tag"      , required_argument, NULL, 't'},
                                             {"directory", required_argument, NULL, OPT_DIRECTORY},
                                             {"extension", required_argument, NULL, OPT_EXTENSION},
                                             {"editor"   , required_argument, NULL, OPT_EDITOR},
                                             {NULL       , 0                , NULL,  0 },
                                            };

    *nonoptsc  = 0;
    *ntags     = 0;
    while ((opt = getopt_long(argc, argv, "-hvt:", opt_list, NULL)) != -1){
        switch (opt){
            case 'h':
                *has_help = true;
                break;
            case 'v':
                *has_version = true;
                break;
            case 't':
                tags[*ntags] = optarg;
                *ntags = *ntags + 1;
                break;
            case OPT_DIRECTORY:
                XFREE(config->dir);
                result = parse_directory(optarg, &config->dir);
                if (result != 0){
                    if (result == FILE_FORMAT_ERROR){
                        fprintf(stderr, "%s: Invalid directory: '%s'\nDirectory must be the absolute path format\n", PACKAGE_NAME, optarg);
                        return FILE_FORMAT_ERROR;
                    } else if (result == WORDEXP_ERROR){
                        fprintf(stderr, "%s: Invalid directory: '%s'\n", PACKAGE_NAME, optarg);
                        return WORDEXP_ERROR;
                    } else if (result == MALLOC_ERROR){
                        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                        return MALLOC_ERROR;
                    } else if (result == INPUT_ERROR){
                        fprintf(stderr, "%s: Invalid input to function parse_directory()\n", PACKAGE_NAME);
                        return UNKNOWN_ERROR;
                    } else{
                        fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                        return UNKNOWN_ERROR;
                    }
                }
                break;
            case OPT_EXTENSION:
                XFREE(config->ext);
                result = ext_validation(optarg);
                if (result != 0){
                    if (result == CHARACTER_NOT_ALLOWED_ERROR){
                        fprintf(stderr, "%s: Invalid extension: '%s'\n"
                                        "Extension must consist of alphabets, numbers, '.', '-', and '_'\n"
                                        "Extension cannot start with '.'", PACKAGE_NAME, optarg);
                        return CHARACTER_NOT_ALLOWED_ERROR;
                    } else{
                        fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                        return UNKNOWN_ERROR;
                    }
                }
                config->ext = strdup(optarg);
                if (config->ext == NULL){
                    return MALLOC_ERROR;
                }
                break;
            case OPT_EDITOR:
                XFREE(config->editor);
                config->editor = strdup(optarg);
                if (config->editor == NULL){
                    return MALLOC_ERROR;
                }
                break;
            case 1:
                if (*nonoptsc == 0 && strcmp(optarg, "git") == 0){
                    *git_pos = optind - 1;
                    return GIT_FOUND;
                }

                nonopts[*nonoptsc] = optarg;
                *nonoptsc = *nonoptsc + 1;
                break;
            case '?':
                return INVALID_OPTION;
        }
    }
    nonopts[*nonoptsc] = NULL;
    tags[*ntags]       = NULL;

    return 0;
}



