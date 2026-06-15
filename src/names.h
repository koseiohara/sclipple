
#include <stdlib.h>
int get_env(const char* env, char** output);
int flag_validation(const char* flag);
void get_filename(const char* flag, char* datetime, char* ext, size_t output_len, char* output);
int mv_filename(char* old_file, const char* new_flag, size_t output_len, char* output);
int path_status(const char* file, struct stat* st);

