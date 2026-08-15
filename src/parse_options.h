
#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include "get_rc.h"

#define SHOW_VERSION (-1)
#define INVALID_OPTION (-2)
#define GIT_FOUND (-3)

int parse_opts(int argc, char **argv,
               int *has_help, int *has_version, int *git_pos,
               int *nonoptsc, char **nonopts,
               int *ntags, char **tags,
               Config *config);

#endif


