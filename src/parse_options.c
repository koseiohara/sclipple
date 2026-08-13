

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "globals.h"
#include "parse_options.h"


int parse_opts(int argc, char** argv, int* has_help, int* has_tag, int* nonoptsc, char** nonopts, int* ntags, char** tags){
    int opt;
    static const struct option opt_list[] = {
                                             {"help"   , no_argument      , NULL, 'h'},
                                             {"version", no_argument      , NULL, 'v'},
                                             {"tag"    , required_argument, NULL, 't'},
                                             {NULL     , 0                , NULL,  0 },
                                            };

    *nonoptsc = 0;
    *ntags    = 0;
    while ((opt = getopt_long(argc, argv, "-hvt:", opt_list, NULL)) != -1){
        switch (opt){
            case 'h':
                *has_help = true;
                break;
            case 'v':
                // has_version = true;
                printf("%s version %s\n", PACKAGE_NAME, PACKAGE_VERSION);
                return SHOW_VERSION;
                // break;
            case 't':
                *has_tag = true;
                tags[*ntags] = optarg;
                *ntags = *ntags + 1;
                break;
            case 1:
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



