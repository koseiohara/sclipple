
#include <stdlib.h>
int get_env(const char* env, char** output);
int check_flag_length(char* flag);
void get_filename(const char* flag, char* datetime, char* ext, size_t output_len, char* output);
void mv_filename(const char* new_flag, char* output);

