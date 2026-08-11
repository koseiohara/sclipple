

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
char* get_element(size_t* line_len, char** line, size_t* ellen);
int parse_meta(char* meta, char** datetime, int* ntags, char*** tags);
int make_meta(char** meta, char* datetime, int ntags, char** tags);
int parse_line(char** line, char** key, char** file, char** meta);
int write_one_line(FILE* fp, ListField field);
int add_contents_to_list(FILE* fp, char* key, char* file, char* datetime, char** tags);
int key_exist_check(FILE* fp, int nkeys, char** keys, char** exist, char** nexist);
int get_content_by_key(char* list, int nkeys, char** keys, ListField** field, int file, int meta);
int get_content_by_tag(char* list, int ntags, char** tags, int* nlines, ListField** field);
int edit_list(char* list, char* mode, int nkeys, char** key, char** info);


