

#include <stdlib.h>
#include <stdio.h>
#include "list_formatter.h"

// int tags2line(int ntags, char** tags, char** line);
int tags2line(int ntags, char* const* tags, char** line);
// int tags_add(int* ntags, char*** updated, char** tags, char** add);
int tags_add(int* ntags, char*** updated, char* const* tags, char* const* add);
// int tags_del(int* ntags, char*** updated, char** tags, char** del);
int tags_del(int* ntags, char*** updated, char* const* tags, char* const* del);
// int fields_add(int* nfields, ListField** updated, int fields_len, ListField* fields, int add_len, ListField* add);
int fields_add(int* nfields, ListField** updated, const int fields_len, const ListField* fields, const int add_len, const ListField* add);
int add_contents_to_list(FILE* fp, char* key, char* file, char* datetime, int ntags, char** tags);
// int key_exist_check(FILE* fp, int nkeys, char** keys, char** exist, char** nexist);
int key_exist_check(FILE* fp, const int nkeys, char* const* keys, char** exist, char** nexist);
// int get_content_by_key(FILE* fp, int nkeys, char** keys, ListField** field, int file, int meta);
int get_content_by_key(FILE* fp, const int nkeys, char* const* keys, ListField** field, const int file, const int meta);
// int get_content_by_tag(FILE* fp, int ntags, char** tags, int* nlines, ListField** field);
int get_content_by_tag(FILE* fp, const int ntags, char* const* tags, int* nlines, ListField** field);
// int get_content_by_key_and_tag(FILE* fp, int nkeys, char** keys, int* found_by_key, char** unfound, int ntags, char** tags, int* found_by_tag, int* found_all, ListField** field, ListField** by_key, ListField** by_tag);
int get_content_by_key_and_tag(FILE* fp, const int nkeys, char* const* keys, int* found_by_key, char** unfound, const int ntags, char* const* tags, int* found_by_tag, int* found_all, ListField** field, ListField** by_key, ListField** by_tag);
// int edit_list(const char* list, char* mode, int nkeys, char** key, char** info);
int edit_list(const char* list, const char* mode, const int nkeys, char* const* keys, char* const* info);


