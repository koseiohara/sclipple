
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "strutils.h"
#include "names.h"
#include "list_utils.h"

#define DELIM ','


void free_ListField(ListField* field){
    int i;
    if (field == NULL){
        return;
    }

    if (field->tags != NULL){
        for (i = 0; i < field->ntags; i = i + 1){
            free(field->tags[i]);
        }
    }

    free(field->key );
    free(field->file);
    free(field->meta);
    free(field->date);
    free(field->tags);

    field->key   = NULL;
    field->file  = NULL;
    field->meta  = NULL;
    field->date  = NULL;
    field->tags  = NULL;
    field->ntags = 0;
}


// return NULL if line is empty or invalid format
// return 0 otherwise
char* get_element(size_t* line_len, char** line, size_t* ellen){
    char*  len_c;
    char*  delim;
    char*  next;
    int    has_line_len;
    size_t header_len;
    size_t work_len;

    if (line_len == NULL){
        has_line_len = false;
    } else{
        has_line_len = true;
    }

    *ellen = -1;

    len_c = *line;
    delim = strchr(len_c, DELIM);
    if (delim == NULL){
        // no delimiter found
        return NULL;
    }

    next = delim + 1;

    if (has_line_len == false){
        work_len = strlen(next);
    } else{
        header_len = (size_t)(next - *line);
        if (*line_len < header_len){
            // invalid line length
            return NULL;
        }

        work_len = *line_len - header_len;
    }

    *delim = '\0';
    *ellen = atoi(len_c);

    if (*ellen > work_len){
        // invalid element length
        return NULL;
    }

    next[*ellen] = '\0';
    *line = next + (*ellen);

    if (*ellen < work_len){
        // the read is not the last column
        *line = *line + 1;

        if (has_line_len == true){
            *line_len = work_len - (1 + (*ellen));
        }
    } else{
        if (has_line_len == true){
            *line_len = 0;
        }
    }

    return next;
}


// return LIST_FORMAT_ERROR if list file is broken
// return MALLOC_ERROR if malloc failed
// return 0 otherwise
int parse_meta(char* meta, char** datetime, int* ntags, char*** tags){
    char*  work_datetime;
    char*  ntags_c;
    int    i;
    size_t line_len;
    size_t ellen;

    line_len = strlen(meta);
    work_datetime = get_element(&line_len, &meta, &ellen);
    if (work_datetime == NULL){
        return LIST_FORMAT_ERROR;
    }

    if (datetime != NULL){
        *datetime = work_datetime;
    }

    if (tags == NULL){
        return 0;
    }

    ntags_c = get_element(&line_len, &meta, &ellen);
    if (ntags_c == NULL){
        *ntags = 0;
        *tags  = NULL;
        return 0;
    }
    *ntags  = atoi(ntags_c);

    *tags = malloc((size_t)(*ntags+1) * sizeof(char*));
    if (*tags == NULL){
        return MALLOC_ERROR;
    }
    for (i = 0; i < *ntags; i = i + 1){
        (*tags)[i] = get_element(&line_len, &meta, &ellen);
        if ((*tags)[i] == NULL){
            return LIST_FORMAT_ERROR;
        }
    }
    (*tags)[*ntags] = NULL;

    return 0;
}


int tags_add(int* ntags, char*** updated, char** tags, char** add){
    int tags_len;
    int add_len;
    int malloc_size;
    int found;
    int i;
    int j;

    if (add == NULL){
        return INPUT_ERROR;
    }

    tags_len = 0;
    while (tags[tags_len] != NULL){
        tags_len = tags_len + 1;
    }

    add_len = 0;
    while(add[add_len] != NULL){
        add_len = add_len + 1;
    }

    malloc_size = tags_len + add_len + 1;
    *updated = malloc((size_t)malloc_size * sizeof(char*));
    if (*updated == NULL){
        return MALLOC_ERROR;
    }

    for (i = 0; i < tags_len; i = i + 1){
        (*updated)[i] = tags[i];
    }

    *ntags = tags_len;
    for (j = 0; j < add_len; j = j + 1){
        found = false;
        for (i = 0; i < *ntags; i = i + 1){
            if (strcmp(add[j], (*updated)[i]) == 0){
                found = true;
                break;
            }
        }
        if (found == false){
            (*updated)[*ntags] = add[j];
            *ntags = *ntags + 1;
        }
    }

    for (i = *ntags; i < malloc_size; i = i + 1){
        (*updated)[i] = NULL;
    }

    return 0;
}


int tags_del(int* ntags, char*** updated, char** tags, char** del){
    int tags_len;
    int del_len;
    int malloc_size;
    int found;
    int i;
    int j;

    if (del == NULL){
        return INPUT_ERROR;
    }

    tags_len = 0;
    while (tags[tags_len] != NULL){
        tags_len = tags_len + 1;
    }

    del_len = 0;
    while(del[del_len] != NULL){
        del_len = del_len + 1;
    }

    malloc_size = tags_len + 1;
    *updated = malloc((size_t)malloc_size * sizeof(char*));
    if (*updated == NULL){
        return MALLOC_ERROR;
    }

    *ntags = 0;
    for (i = 0; i < tags_len; i = i + 1){
        found = false;
        for (j = 0; j < del_len; j = j + 1){
            if (strcmp(del[j], tags[i]) == 0){
                found = true;
                break;
            }
        }
        if (found == false){
            (*updated)[*ntags] = tags[i];
            *ntags = *ntags + 1;
        }
    }

    for (i = *ntags; i < malloc_size; i = i + 1){
        (*updated)[i] = NULL;
    }

    return 0;
}


// ntags is ignored if tags == NULL
//
// return INPUT_ERROR if one or more arguments are invalid
// return IO_ERROR if io failed
// return MALLOC_ERROR if malloc failed
// return 0 otherwise
int make_meta(char** meta, char* datetime, int ntags, char** tags){
    FILE* fp = NULL;
    int result;
    int ret;
    int i;
    size_t meta_size;
    size_t datetime_size;
    size_t ntags_size;
    size_t tags_size;

    if (meta == NULL || *meta != NULL){
        ret = INPUT_ERROR;
        goto cleanup;
    }

    if (ntags <= 0 && tags != NULL){
        ret = INPUT_ERROR;
        goto cleanup;
    }

    fp = open_memstream(meta, &meta_size);
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    datetime_size = strlen(datetime);
    result = fprintf(fp, "%zu%c%s", datetime_size, DELIM, datetime);
    if (result < 0){
        if (errno == ENOMEM){
            ret = MALLOC_ERROR;
        } else{
            ret = IO_ERROR;
        }
        goto cleanup;
    }

    if (tags != NULL){
        result = snprintf(NULL, 0, "%d", ntags);
        if (result < 0){
            ret = IO_ERROR;
            goto cleanup;
        }
        ntags_size = (size_t)result;

        if (datetime != NULL){
            result = fputc(',', fp);
            if (result == EOF){
                ret = IO_ERROR;
                goto cleanup;
            }
        }

        result = fprintf(fp, "%zu%c%d%c", ntags_size, DELIM, ntags, DELIM);
        if (result < 0){
            if (errno == ENOMEM){
                ret = MALLOC_ERROR;
            } else{
                ret = IO_ERROR;
            }
            goto cleanup;
        }

        for (i = 0; i < ntags; i = i + 1){
            if (tags[i] == NULL){
                ret = INPUT_ERROR;
                goto cleanup;
            }
            tags_size = strlen(tags[i]);
            if (i < ntags-1){
                result = fprintf(fp, "%zu%c%s%c", tags_size, DELIM, tags[i], DELIM);
            } else{
                result = fprintf(fp, "%zu%c%s"  , tags_size, DELIM, tags[i]);
            }
            if (result < 0){
                if (errno == ENOMEM){
                    ret = MALLOC_ERROR;
                } else{
                    ret = IO_ERROR;
                }
                goto cleanup;
            }
        }
    }

    ret = 0;
    goto cleanup;


cleanup:
    if (xfclose(&fp) != 0){
        if (ret == 0){
            ret = IO_ERROR;
        }
    }
    return ret;
}


// return LIST_FORMAT_ERROR if list file is broken
// return 0 otherwise
int parse_line(char** line, char** key, char** file, char** meta){
    char*  work_file;
    int    ret;
    size_t line_len;
    size_t ellen;

    (*line)[strcspn(*line, "\n")] = '\0';

    line_len = strlen(*line);
    *key = get_element(&line_len, line, &ellen);
    if (*key == NULL){
        ret = LIST_FORMAT_ERROR;
        goto cleanup;
    }

    if (file == NULL && meta == NULL){
        ret = 0;
        goto cleanup;
    }

    work_file = get_element(&line_len, line, &ellen);
    if (work_file == NULL){
        ret = LIST_FORMAT_ERROR;
        goto cleanup;
    }
    if (file != NULL){
        *file = work_file;
    }

    if (meta != NULL){
        if (line_len == 0){
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
        }
        *meta = *line;
        // *meta = get_element(&line_len, line, &ellen);
        // if (*meta == NULL){
        //     ret = LIST_FORMAT_ERROR;
        //     goto cleanup;
        // }
    }

    ret = 0;
    goto cleanup;


cleanup:
    return ret;
}


// date, tags, and ntags are ignored
//
// return IO_ERROR if IO failed
// return 0 otherwise
int write_one_line(FILE* fp, ListField field){
    int result;
    int ret;

    result = fprintf(fp, "%zu%c%s%c%zu%c%s%c%s\n", strlen(field.key) , DELIM, field.key , DELIM, 
                                                   strlen(field.file), DELIM, field.file, DELIM, 
                                                   field.meta);
    if (result < 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    return ret;
}


// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc failed
// return unknown_error if a bug is found
// return 0 otherwise
int add_contents_to_list(FILE* fp, char* key, char* file, char* datetime, char** tags){
    // FILE* fp   = NULL;
    ListField field = {0};
    char* meta = NULL;
    int   result;
    int   ret;
    int   ntags;
    int   i;

    ntags = 0;
    if (tags != NULL){
        while (tags[ntags] != NULL){
            ntags = ntags + 1;
        }
    }

    result = make_meta(&meta, datetime, ntags, tags);
    if (result != 0){
        if (result == IO_ERROR){
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            ret = MALLOC_ERROR;
        } else{
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    field = (ListField){
        .key  = key,
        .file = file,
        .meta = meta,
        .date = NULL,
        .tags = NULL,
        .ntags = 0,
    };

    result = write_one_line(fp, field);
    if (result != 0){
        if (result == IO_ERROR){
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            ret = MALLOC_ERROR;
        } else{
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    free(meta);
    return ret;
}


// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc_failed
// return UNKNOWN_ERROR if a bug is found
// return KEY_NOT_FOUND if one or more keys were not found
// return 0 if all keys exist
int key_exist_check(FILE* fp, int nkeys, char** keys, char** exist, char** nexist){
    // FILE*  fp   = NULL;
    char*  ikey = NULL;
    char*  line = NULL;
    char*  work_line;
    int*   key_is_exist = NULL;
    int    result;
    int    ret;
    int    i;
    int    exist_count;
    int    nexist_count;
    int    found;
    size_t size;

    // fp = fopen(list, "r");
    // if (fp == NULL){
    //     ret = IO_ERROR;
    //     goto cleanup;
    // }

    key_is_exist = malloc((size_t)nkeys * sizeof(int));
    if (key_is_exist == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
    }
    for (i = 0; i < nkeys; i = i + 1){
        key_is_exist[i] = false;
        if (exist != NULL){
            exist[i]  = NULL;
        }
        if (nexist != NULL){
            nexist[i] = NULL;
        }
    }

    exist_count = 0;
    while (getline(&line, &size, fp) != -1){
        work_line = line;
        result = parse_line(&work_line, &ikey, NULL, NULL);
        if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                ret = LIST_FORMAT_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }

        for (i = 0; i < nkeys; i = i + 1){
            if (strcmp(ikey, keys[i]) == 0){
                if (key_is_exist[i] == false){
                    key_is_exist[i] = true;
                    exist_count = exist_count + 1;
                }
                break;
            }
        }
        if (exist_count == nkeys){
            // if all of keys are found, skip reading the remaining lines
            if (exist != NULL){
                for (i = 0; i < nkeys; i = i + 1){
                    exist[i] = keys[i];
                }
            }
            ret = 0;
            goto cleanup;
        }
    }

    if (ferror(fp) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    exist_count  = 0;
    nexist_count = 0;
    ret = 0;
    for (i = 0; i < nkeys; i = i + 1){
        if (key_is_exist[i] == true){
            if (exist != NULL){
                exist[exist_count]   = keys[i];
                exist_count = exist_count + 1;
            }
        } else{
            if (nexist != NULL){
                nexist[nexist_count] = keys[i];
                nexist_count = nexist_count + 1;
            }
            ret = KEY_NOT_FOUND;
        }
    }

    goto cleanup;

cleanup:
    // if (xfclose(&fp) != 0){
    //     if (ret == 0){
    //         ret = IO_ERROR;
    //     }
    // }
    free(line);
    free(key_is_exist);
    return ret;
}


// if files == NULL, files will not be read. similarly, meta will not be read if meta == NULL
// value is NULL if the key is not found
//
// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR if a bug is found
// return 0 otherwise
int get_content_by_key(char* list, int nkeys, char** keys, ListField** field, int file, int meta){
    FILE*  fp   = NULL;
    char*  line = NULL;
    char*  work_key;
    char*  work_file;
    char*  work_meta;
    char*  work_line;
    int    result;
    int    ret;
    int    i;
    size_t size;

    fp = fopen(list, "r");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    if (*field == NULL){
        *field = malloc((size_t)nkeys * sizeof(ListField));
        if (*field == NULL){
            ret = MALLOC_ERROR;
            goto cleanup;
        }
    }
    for (i = 0; i < nkeys; i = i + 1){
        (*field)[i] = (ListField){0};
    }

    while (getline(&line, &size, fp) != -1){
        work_line = line;
        result = parse_line(&work_line, &work_key, &work_file, &work_meta);
        if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                ret = LIST_FORMAT_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }

        for (i = 0; i < nkeys; i = i + 1){
            if (strcmp(work_key, keys[i]) == 0){
                work_line = line;
                // result = parse_line(&work_line, &work_key, &work_file, &work_meta);
                free_ListField(&(*field)[i]);

                (*field)[i].key = strdup(work_key);
                if ((*field)[i].key == NULL){
                    ret = MALLOC_ERROR;
                    goto cleanup;
                }

                if (file == true){
                    (*field)[i].file = strdup(work_file);
                    if ((*field)[i].file == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                }
                if (meta == true){
                    (*field)[i].meta = strdup(work_meta);
                    if ((*field)[i].meta == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                }
                break;
            }
        }
    }

    if (ferror(fp) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    xfclose(&fp);
    free(line);
    return ret;
}


// return INPUT_ERROR if an argument is invalid
// return UNKNOWN_ERROR if a bug is found
// return IO_ERROR if io failed
// return MALLOC_ERROR if malloc_failed
// return LIST_FORMAT_ERROR if list file is broken
// return 0 otherwise
int get_content_by_tag(char* list, int ntags, char** tags, 
                       int* nlines, ListField** field){
    FILE*      fp   = NULL;
    ListField* p    = NULL;
    char*      line = NULL;
    char*  work_line;
    char*  work_key;
    char*  work_file;
    char*  work_meta;
    char*  work_datetime;
    char** work_tags = NULL;
    int    work_ntags;
    int    result;
    int    ret;
    int    found;
    int    i;
    int    j;
    int    k;
    int    capacity;
    size_t size;

    fp = fopen(list, "r");
    if (fp == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    if (*field != NULL){
        ret = INPUT_ERROR;
        goto cleanup;
    }

    capacity = 2;
    *field = malloc((size_t)capacity * sizeof(ListField));
    if (*field == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
    }
    for (i = 0; i < capacity; i = i + 1){
        (*field)[i] = (ListField){0};
    }

    *nlines = 0;
    while (getline(&line, &size, fp) != -1){
        work_line = line;
        result = parse_line(&work_line, &work_key, &work_file, &work_meta);
        if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                ret = LIST_FORMAT_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }

        result = parse_meta(work_meta, &work_datetime, &work_ntags, &work_tags);
        if (result != 0){
            if (result == LIST_FORMAT_ERROR){
                ret = LIST_FORMAT_ERROR;
            } else if (result == MALLOC_ERROR){
                ret = MALLOC_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }

        if (work_ntags == 0){
            continue;
        }

        found = false;
        for (i = 0; i < work_ntags; i = i + 1){
            for (j = 0; j < ntags; j = j + 1){
                if (strcmp(tags[j], work_tags[i]) == 0){
                    found = true;
                    if (*nlines == capacity){
                        capacity = capacity << 1;
                        p = realloc(*field, (size_t)capacity * sizeof(ListField));
                        if (p == NULL){
                            ret = MALLOC_ERROR;
                            goto cleanup;
                        }
                        *field = p;
                        for (k = *nlines; k < capacity; k = k + 1){
                            (*field)[k] = (ListField){0};
                        }
                    }
                    (*field)[*nlines] = (ListField){
                        .key   = strdup(work_key),
                        .file  = strdup(work_file),
                        .meta  = NULL,
                        .date  = strdup(work_datetime),
                        .ntags = work_ntags,
                        .tags  = malloc((size_t)work_ntags * sizeof(char*)),
                    };
                    if ((*field)[*nlines].key == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    if ((*field)[*nlines].file == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    if ((*field)[*nlines].date == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    if ((*field)[*nlines].tags == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    for (k = 0; k < work_ntags; k = k + 1){
                        (*field)[*nlines].tags[k] = strdup(work_tags[k]);
                    }
                    *nlines = *nlines + 1;
                    break;
                }
            }
            if (found == true){
                break;
            }
        }
        XFREE(work_tags);
    }

    if (ferror(fp) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;

cleanup:
    xfclose(&fp);
    free(line);
    free(work_tags);
    return ret;
}


// info...
//     if mode="mv"  , keys can have only 1 element. info[0] is the new key and info[1] is the new file name. info[n]; n>2 are ignored
//     if mode="rm"  , keys can have only 1 element. info must be NULL
//     if mode="tag" , info is new tags
//     if mode="utag", info is tags to be deleted
//
// return INPUT_ERROR if an argument is invalid
// return UNKNOWN_ERROR if a bug is found
// return IO_ERROR if IO failed
// return RENAME_ERROR rename failed
// return MALLOC_ERROR malloc failed
// return LIST_FORMAT_ERROR list file is broken
// return KEY_DUPLICATE if new_key already exist
// return 0 otherwise
int edit_list(char* list, char* mode, int nkeys, char** keys, char** info){
    const int mode_rm   = 1;
    const int mode_mv   = 2;
    const int mode_tag  = 3;
    const int mode_utag = 4;
    FILE*     fpr  = NULL;
    FILE*     fpw  = NULL;
    ListField field     = {0};
    char*     line      = NULL;
    char*     tmpfile   = NULL;
    char*     work_tags = NULL;
    char*     work_line;
    char*     work_key;
    char*     work_file;
    char*     work_meta;
    char**    work_exist;
    char**    work_nexist;
    int*      is_found = NULL;
    int       imode;
    int       fd;
    int       result;
    int       ret;
    int       work_nkeys;
    int       changed;
    int       i;
    size_t    size;
    struct stat st;

    changed = false;

    if (stat(list, &st) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    if (strcmp(mode, "mv") == 0){
        imode = mode_mv;
        if (info != NULL){
            ret = INPUT_ERROR;
            goto cleanup;
        }
    } else if (strcmp(mode, "rm") == 0){
        imode = mode_rm;
        if (nkeys != 1 || info != NULL){
            ret = INPUT_ERROR;
            goto cleanup;
        }
    } else if (strcmp(mode, "tag") == 0){
        imode = mode_tag;
    } else if (strcmp(mode, "utag") == 0){
        imode = mode_utag;
    } else{
        ret = INPUT_ERROR;
        goto cleanup;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    // check existence of the new flag
    if (imode == mode_mv){
        result = key_exist_check(fpr, 1, info, NULL, NULL);
        if (result != KEY_NOT_FOUND){
            if (result == 0){
                ret = KEY_DUPLICATE;
            } else if (result == IO_ERROR){
                ret = IO_ERROR;
            } else if (result == MALLOC_ERROR){
                ret = MALLOC_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }
        rewind(fpr);
    } else if (imode == mode_tag || imode == mode_utag){
        result = key_exist_check(fpr, 1, keys, work_exist, NULL);
        if (result == 0){
            work_nkeys = nkeys;
        } else if (result == KEY_NOT_FOUND){
            work_nkeys = 0;
            while (work_exist[work_nkeys] != NULL){
                work_nkeys = work_nkeys + 1;
            }
        } else{
            if (result == IO_ERROR){
                ret = IO_ERROR;
            } else if (result == MALLOC_ERROR){
                ret = MALLOC_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }
        rewind(fpr);

        is_found = malloc((size_t)nkeys * sizeof(int));
        for (i = 0; i < nkeys; i = i + 1){
            is_found[i] = false;
        }
    }

    result = asprintf(&tmpfile, "%s.XXXXXX", list);
    if (result < 0){
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    fd = mkstemp(tmpfile);
    if (fd == -1){
        ret = IO_ERROR;
        goto cleanup;
    }

    fpw = fdopen(fd, "w");
    if (fpw == NULL){
        unlink(tmpfile);
        close(fd);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (fchmod(fd, st.st_mode) != 0){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    while (getline(&line, &size, fpr) != -1){
        if (is_white_space(line) == true){
            XFREE(line);
            continue;
        }

        work_line = line;
        result = parse_line(&work_line, &work_key, &work_file, &work_meta);
        if (result != 0){
            unlink(tmpfile);
            if (result == LIST_FORMAT_ERROR){
                ret = LIST_FORMAT_ERROR;
            } else{
                ret = UNKNOWN_ERROR;
            }
            goto cleanup;
        }

        if (imode == mode_rm){
            if (strcmp(work_key, *keys) == 0){
                // XFREE(line);
                changed = true;
                // continue;
            } else{
                field = (ListField){
                    .key   = work_key,
                    .file  = work_file,
                    .meta  = work_meta,
                    .date  = NULL,
                    .tags  = NULL,
                    .ntags = 0,
                };
                result = write_one_line(fpw, field);
                if (result != 0){
                    unlink(tmpfile);
                    if (result == IO_ERROR){
                        ret = IO_ERROR;
                    } else{
                        ret = UNKNOWN_ERROR;
                    }
                    goto cleanup;
                }
            }
        } else if (imode == mode_mv){
            if (strcmp(work_key, *keys) == 0){
                field = (ListField){
                    .key   = info[0],
                    .file  = info[1],
                    .meta  = work_meta,
                    .date  = NULL,
                    .tags  = NULL,
                    .ntags = 0,
                };
                changed = true;
            } else{
                field = (ListField){
                    .key   = work_key,
                    .file  = work_file,
                    .meta  = work_meta,
                    .date  = NULL,
                    .tags  = NULL,
                    .ntags = 0,
                };
            }
            result = write_one_line(fpw, field);
            if (result != 0){
                unlink(tmpfile);
                if (result == IO_ERROR){
                    ret = IO_ERROR;
                } else{
                    ret = UNKNOWN_ERROR;
                }
                goto cleanup;
            }
        } else if (imode == mode_tag || imode == mode_utag){
            for (i = 0; i < work_nkeys; i = i + 1){
                if (is_found[i] == false && strcmp(work_key, work_exist[i]) == 0){
                    is_found[i] = true;
                    break;
                }
            }
        }
        XFREE(line);
    }

    if (changed == false){
        unlink(tmpfile);
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }

    if (ferror(fpr) != 0){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (xfclose(&fpr)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (xfclose(&fpw)){
        unlink(tmpfile);
        ret = IO_ERROR;
        goto cleanup;
    }

    if (rename(tmpfile, list) != 0){
        unlink(tmpfile);
        ret = RENAME_ERROR;
        goto cleanup;
    }
    
    XFREE(tmpfile);

    ret = 0;
    goto cleanup;


cleanup:
    xfclose(&fpr);
    xfclose(&fpw);

    free(line);
    free(is_found);
    free(tmpfile);

    return ret;
}



