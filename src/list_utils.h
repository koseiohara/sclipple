
#ifndef _LIST_UTILS_H
#define _LIST_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include "list_formatter.h"

int tags_add(int* ntags, char*** updated, char* const* tags, char* const* add);
int tags_del(int* ntags, char*** updated, char* const* tags, char* const* del);
int fields_add(int* nfields, ListField** updated, const int fields_len, const ListField* fields, const int add_len, const ListField* add);
int add_contents_to_list(FILE* fp, char* key, char* file, char* datetime, int ntags, char** tags);
int key_exist_check(FILE* fp, const int nkeys, char* const* keys, char** exist, char** nexist);
int get_content_by_key(FILE* fp, const char* dir, const int nkeys, char* const* keys, ListField** field, const int file, const int meta);
int get_content_by_tag(FILE* fp, const char* dir, const char* match, const int ntags, char* const* tags, int* nlines, ListField** field);
int get_content_by_key_and_tag(FILE* fp, const char* dir, const char* match,
                               const int nkeys, char* const* keys, int* found_by_key, char** unfound,
                               const int ntags, char* const* tags, int* found_by_tag,
                               int* found_all, ListField** field, ListField** by_key, ListField** by_tag);
int edit_list(const char* list, const char* mode, const int nkeys, char* const* keys, char* const* info);

#endif

