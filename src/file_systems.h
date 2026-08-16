
#ifndef _FILE_SYSTEMS_H
#define _FILE_SYSTEMS_H

#include <stdlib.h>
#include <sys/stat.h>

int get_env(const char* env, char** output);
int parse_directory(const char* input_dir, char** output_dir);
int get_filename(const char* key, char* ext, char** output);
int file_to_abs(const char* dir, const char* file, char** output);
char* abs_to_file(char* abs);
int mv_filename(char* old_file, const char* new_key, char** output);
int path_status(const char* file, struct stat* st);
int make_dir(const char* dir);
int make_file(const char* path, const int cond);

#endif

