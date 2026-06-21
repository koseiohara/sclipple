
#include <stdlib.h>
#include <sys/stat.h>

int get_env(const char* env, char** output);
int ext_validation(const char* ext);
int flag_validation(const char* flag);
int parse_directory(const char* input_dir, char** output_dir);
int get_filename(const char* flag, char* datetime, char* ext, char** output);
int mv_filename(char* old_file, const char* new_flag, char** output);
int path_status(const char* file, struct stat* st);

