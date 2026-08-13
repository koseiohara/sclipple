

#include <stdlib.h>
#include <stdio.h>

typedef struct{
    char*  key;
    char*  file;
    char*  meta;
    char*  date;
    char** tags;
    int    ntags;
} ListField;


void free_ListField(ListField* field);
// char* get_element(size_t* line_len, char** line, size_t* ellen);
int parse_meta(char* meta, char** datetime, int* ntags, char*** tags);
int make_meta(char** meta, const char* datetime, const int ntags, char* const* tags);
// int make_meta(char** meta, char* datetime, int ntags, char** tags);
int parse_line(char** line, char** key, char** file, char** meta);
int write_one_line(FILE* fp, ListField field);

