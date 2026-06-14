
#include <stdlib.h>
#include <stdio.h>

int is_white_space(const char* line);
int write_new_content_to_list(const char* list, const char* flag, const char* datetime, const char* file);
int flag_exist_check(const char* list, char* flag);
int get_datetime_by_key(const char* list, char* flag, char* datetime);
int get_filename_by_key(const char* list, char* flag, char* filename);
int mv_key_in_list(const char* list, const char* old_flag, const char* new_flag);
int rm_key_in_list(const char* list, const char* target_flag);
int get_content_line(FILE* fp, size_t flag_len, char* flag, size_t datetime_len, char* datetime, size_t notename_len, char* notename);

