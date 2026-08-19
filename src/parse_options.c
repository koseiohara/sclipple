

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
    OPT_TAGMATCH,
};


// return MALLOC_ERROR if malloc failed
// return FILE_FORMAT_ERROR if input directory does not start with /
// return CHARACTER_NOT_ALLOWED_ERROR if invalid character is included in extension
// return INVALID_OPTION if unknown option is obtained
// return UNKNOWN_ERROR if a bug is found
// return GIT_FOUND if the first non-option argument is git
// return 0 otherwise
int parse_opts(int argc, char** argv, int* has_help, int* has_version, int* has_short, int* git_pos, int* nonoptsc, char** nonopts, int* ntags, char** tags, Config* config){
    char* directory;
    char* extension;
    char* editor;
    char* tag_match;
    int opt;
    int result;
    static const struct option opt_list[] = {
                                             {"help"     , no_argument      , NULL, 'h'},
                                             {"version"  , no_argument      , NULL, 'v'},
                                             {"short"    , no_argument      , NULL, 's'},
                                             {"tag"      , required_argument, NULL, 't'},
                                             {"directory", required_argument, NULL, OPT_DIRECTORY},
                                             {"extension", required_argument, NULL, OPT_EXTENSION},
                                             {"editor"   , required_argument, NULL, OPT_EDITOR},
                                             {"tag-match", required_argument, NULL, OPT_TAGMATCH},
                                             {NULL       , 0                , NULL,  0 },
                                            };

    *has_help    = false;
    *has_version = false;
    *has_short   = false;
    *nonoptsc = 0;
    *ntags    = 0;
    *git_pos  = -1;
    directory = NULL;
    extension = NULL;
    editor    = NULL;
    tag_match = NULL;
    while ((opt = getopt_long(argc, argv, "-hvst:", opt_list, NULL)) != -1){
        switch (opt){
            case 'h':
                *has_help = true;
                break;
            case 'v':
                *has_version = true;
                break;
            case 's':
                *has_short = true;
                break;
            case 't':
                tags[*ntags] = optarg;
                *ntags = *ntags + 1;
                break;
            case OPT_DIRECTORY:
                directory = optarg;
                break;
            case OPT_EXTENSION:
                extension = optarg;
                break;
            case OPT_EDITOR:
                editor = optarg;
                break;
            case OPT_TAGMATCH:
                tag_match = optarg;
                break;
            case 1:
                if (*nonoptsc == 0 && strcmp(optarg, "git") == 0){
                    *git_pos = optind - 1;
                }

                nonopts[*nonoptsc] = optarg;
                *nonoptsc = *nonoptsc + 1;
                break;
            case '?':
                return INVALID_OPTION;
        }
        if (*git_pos >= 0){
            break;
        }
    }

    if (*has_version == true || *has_help == true){
        return 0;
    }

    if (directory != NULL){
        XFREE(config->dir);
        if (directory[0] != '/'){
            fprintf(stderr, "%s: Invalid directory: '%s'\n"
                            "Directory must be the absolute path format\n", PACKAGE_NAME, directory);
            return ARG_ERROR;
        }
        config->dir = strdup(directory);
        if (config->dir == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            return MALLOC_ERROR;
        }
    }

    if (extension != NULL){
        XFREE(config->ext);
        result = ext_validation(extension);
        if (result != 0){
            if (result == CHARACTER_NOT_ALLOWED_ERROR){
                fprintf(stderr, "%s: Invalid extension: '%s'\n"
                                "Extension must consist of alphabets, numbers, '.', '-', and '_'\n"
                                "Extension cannot start with '.'\n", PACKAGE_NAME, extension);
                return ARG_ERROR;
            } else{
                fprintf(stderr, "%s: Unknown Error\n", PACKAGE_NAME);
                return UNKNOWN_ERROR;
            }
        }
        config->ext = strdup(extension);
        if (config->ext == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            return MALLOC_ERROR;
        }
    }

    if (editor != NULL){
        XFREE(config->editor);
        config->editor = strdup(editor);
        if (config->editor == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            return MALLOC_ERROR;
        }
    }

    if (tag_match != NULL){
        if (strcmp(tag_match, "and") != 0 && strcmp(tag_match, "or") != 0){
            fprintf(stderr, "%s: Invalid tag-match input: %s\n"
                            "tag-match must be 'and' or 'or'\n", PACKAGE_NAME, tag_match);
            return ARG_ERROR;
        }

        XFREE(config->tag_match);
        config->tag_match = strdup(tag_match);
        if (config->tag_match == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            return MALLOC_ERROR;
        }
    }

    nonopts[*nonoptsc] = NULL;
    tags[*ntags]       = NULL;

    if (*git_pos >= 0){
        return GIT_FOUND;
    } else{
        return 0;
    }
}



