
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
#include "file_systems.h"
#include "list_utils.h"

#define DELIM ','


// return INPUT_ERROR if an argument is invalid
// resutn MALLOC_ERROR if malloc failed
// resutn 0 otherwise
int tags_add(int* ntags, char*** updated, char* const* tags, char* const* add){
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
    if (tags != NULL){
        while (tags[tags_len] != NULL){
            tags_len = tags_len + 1;
        }
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


int tags_del(int* ntags, char*** updated, char* const* tags, char* const* del){
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
    if (tags != NULL){
        while (tags[tags_len] != NULL){
            tags_len = tags_len + 1;
        }
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


// return INPUT_ERROR if an argument is invalid
// return MALLOCERROR if malloc failed
// return 0 otherwise
int fields_add(int* nfields, ListField** updated, const int fields_len, const ListField* fields, const int add_len, const ListField* add){
    int malloc_size;
    int found;
    int i;
    int j;

    if (nfields == NULL || updated == NULL){
        return INPUT_ERROR;
    }
    if (fields_len < 0 || add_len < 0){
        return INPUT_ERROR;
    }
    if (fields_len > 0 && fields == NULL){
        return INPUT_ERROR;
    }
    if (add_len > 0 && add == NULL){
        return INPUT_ERROR;
    }

    *nfields = 0;
    *updated = NULL;

    malloc_size = fields_len + add_len;
    if (malloc_size == 0){
        return 0;
    }

    *updated = malloc((size_t)malloc_size * sizeof(ListField));
    if (*updated == NULL){
        return MALLOC_ERROR;
    }

    for (i = 0; i < fields_len; i = i + 1){
        if (fields[i].key == NULL){
            continue;
        }

        (*updated)[*nfields] = fields[i];
        *nfields = *nfields + 1;
    }

    for (i = 0; i < add_len; i = i + 1){
        if (add[i].key == NULL){
            continue;
        }

        found = false;
        for (j = 0; j < *nfields; j = j + 1){
            if (strcmp(add[i].key, (*updated)[j].key) == 0){
                found = true;
                break;
            }
        }

        if (found == false){
            (*updated)[*nfields] = add[i];
            *nfields = *nfields + 1;
        }
    }

    return 0;
}


// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc failed
// return UNKNOWN_ERROR if a bug is found
// return 0 otherwise
int add_contents_to_list(FILE* fp, char* key, char* file, char* datetime, int ntags, char** tags){
    ListField field = {0};
    char* meta = NULL;
    int   result;
    int   ret;

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


// return LIST_FORMAT_ERROR if list file is broken
// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc_failed
// return UNKNOWN_ERROR if a bug is found
// return KEY_NOT_FOUND if one or more keys were not found
// return 0 if all keys exist
int key_exist_check(FILE* fp, const int nkeys, char* const* keys, char** exist, char** nexist){
    char*  ikey = NULL;
    char*  line = NULL;
    char*  work_line;
    int*   key_is_exist = NULL;
    int    result;
    int    ret;
    int    i;
    int    exist_count;
    int    nexist_count;
    size_t size = 0;

    if (nkeys <= 0){
        if (nkeys == 0){
            ret = 0;
            goto cleanup;
        }
        ret = INPUT_ERROR;
        goto cleanup;
    }

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

    if (ferror(fp)){
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
    free(line);
    free(key_is_exist);
    return ret;
}


// if file == false, files will not be read. similarly, meta will not be read if meta == false
// value is NULL if the key is not found
//
// return IO_ERROR if IO failed
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR if a bug is found
// return 0 otherwise
int get_content_by_key(FILE* fp, const int nkeys, char* const* keys, ListField** field, const int file, const int meta){
    char*  line = NULL;
    char*  work_key;
    char*  work_file;
    char*  work_meta;
    char*  work_line;
    int    result;
    int    ret;
    int    i;
    size_t size = 0;

    if (nkeys == 0){
        ret = 0;
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
                } else{
                    (*field)[i].file = NULL;
                }

                if (meta == true){
                    (*field)[i].meta = strdup(work_meta);
                    if ((*field)[i].meta == NULL){
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                } else{
                    (*field)[i].meta = NULL;
                }
            }
        }
    }

    if (ferror(fp)){
        ret = IO_ERROR;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;


cleanup:
    free(line);
    return ret;
}


// return INPUT_ERROR if an argument is invalid
// return UNKNOWN_ERROR if a bug is found
// return IO_ERROR if io failed
// return MALLOC_ERROR if malloc_failed
// return LIST_FORMAT_ERROR if list file is broken
// return 0 otherwise
int get_content_by_tag(FILE* fp, const int ntags, char* const* tags, int* nlines, ListField** field){
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
    size_t size = 0;

    if (ntags == 0){
        ret = 0;
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
            XFREE(work_tags);
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
                        .tags  = calloc((size_t)work_ntags, sizeof(char*)),
                    };
                    if ((*field)[*nlines].key == NULL){
                        free_ListField(&(*field)[*nlines]);
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    if ((*field)[*nlines].file == NULL){
                        free_ListField(&(*field)[*nlines]);
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    if ((*field)[*nlines].date == NULL){
                        free_ListField(&(*field)[*nlines]);
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    if ((*field)[*nlines].tags == NULL){
                        free_ListField(&(*field)[*nlines]);
                        ret = MALLOC_ERROR;
                        goto cleanup;
                    }
                    for (k = 0; k < work_ntags; k = k + 1){
                        (*field)[*nlines].tags[k] = strdup(work_tags[k]);
                        if ((*field)[*nlines].tags[k] == NULL){
                            free_ListField(&(*field)[*nlines]);
                            ret = MALLOC_ERROR;
                            goto cleanup;
                        }
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

    if (ferror(fp)){
        ret = IO_ERROR;
        goto cleanup;
    }

    ret = 0;
    goto cleanup;

cleanup:
    free(line);
    free(work_tags);
    return ret;
}


// return IO_ERROR if io failed
// return MALLOC_ERROR if malloc failed
// return LIST_FORMAT_ERROR if list file is broken
// return UNKNOWN_ERROR if a bug is found
// return 0 otherwise
int get_content_by_key_and_tag(FILE* fp, const int nkeys, char* const* keys, int* found_by_key, char** unfound,
                               const int ntags, char* const* tags, int* found_by_tag,
                               int* found_all, ListField** field, ListField** by_key, ListField** by_tag){
    int unfound_count;
    int result;
    int ret;
    int i;

    result = get_content_by_key(fp, nkeys, keys, by_key, true, true);
    if (result != 0){
        if (result == LIST_FORMAT_ERROR){
            ret = LIST_FORMAT_ERROR;
        } else if (result == IO_ERROR){
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            ret = MALLOC_ERROR;
        } else{
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    *found_by_key = 0;
    unfound_count = 0;
    for (i = 0; i < nkeys; i = i + 1){
        if ((*by_key)[i].key != NULL){
            *found_by_key = *found_by_key + 1;
        } else{
            unfound[unfound_count] = keys[i];
            unfound_count = unfound_count + 1;
        }
    }
    for (i = unfound_count; i < nkeys; i = i + 1){
        unfound[i] = NULL;
    }

    if (ntags == 0){
        *found_all = 0;
        if (*found_by_key == 0){
            *field = NULL;
            ret = 0;
            goto cleanup;
        }

        *field = malloc((size_t)(*found_by_key) * sizeof(ListField));
        if (*field == NULL){
            ret = MALLOC_ERROR;
            goto cleanup;
        }

        for (i = 0; i < nkeys; i = i + 1){
            if ((*by_key)[i].key == NULL){
                continue;
            }

            (*field)[*found_all] = (*by_key)[i];
            *found_all = *found_all + 1;
        }
        ret = 0;
        goto cleanup;
    }

    rewind(fp);
    result = get_content_by_tag(fp, ntags, tags, found_by_tag, by_tag);
    if (result != 0){
        if (result == LIST_FORMAT_ERROR){
            ret = LIST_FORMAT_ERROR;
        } else if (result == IO_ERROR){
            ret = IO_ERROR;
        } else if (result == MALLOC_ERROR){
            ret = MALLOC_ERROR;
        } else if (result == INPUT_ERROR){
            ret = UNKNOWN_ERROR;
        } else{
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    result = fields_add(found_all, field, nkeys, *by_key, *found_by_tag, *by_tag);
    if (result != 0){
        if (result == MALLOC_ERROR){
            ret = MALLOC_ERROR;
        } else if (result == INPUT_ERROR){
            ret = UNKNOWN_ERROR;
        } else{
            ret = UNKNOWN_ERROR;
        }
        goto cleanup;
    }

    return 0;
    goto cleanup;


cleanup:
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
// // return KEY_DUPLICATE if new_key already exist
// return KEY_NOT_FOUND if old_key already exist
// return 0 otherwise
int edit_list(const char* list, const char* mode, const int nkeys, char* const* keys, char* const* info){
    const int mode_rm   = 1;
    const int mode_mv   = 2;
    const int mode_tag  = 3;
    const int mode_utag = 4;
    FILE*     fpr  = NULL;
    FILE*     fpw  = NULL;
    ListField field     = {0};
    char*     line      = NULL;
    char*     tmpfile   = NULL;
    char*     work_line;
    char*     work_key;
    char*     work_file;
    char*     work_meta;
    char*     work_datetime;
    char**    work_curr_tags = NULL;
    char**    work_new_tags  = NULL;
    char*     work_new_meta  = NULL;
    int*      is_found = NULL;
    int       imode;
    int       fd;
    int       result;
    int       ret = 0;
    int       work_ntags;
    int       changed;
    int       line_changed;
    int       i;
    int       count;
    size_t    size = 0;
    struct stat st;

    changed = false;

    if (stat(list, &st) != 0){
        ret = IO_ERROR;
        goto cleanup;
    }

    if (nkeys <= 0 || keys == NULL || keys[0] == NULL){
        ret = INPUT_ERROR;
        goto cleanup;
    }

    if (strcmp(mode, "mv") == 0){
        imode = mode_mv;
        if (nkeys != 1 || info == NULL || info[0] == NULL || info[1] == NULL){
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
        if (info == NULL || info[0] == NULL){
            ret = INPUT_ERROR;
            goto cleanup;
        }
    } else if (strcmp(mode, "utag") == 0){
        imode = mode_utag;
        if (info == NULL || info[0] == NULL){
            ret = INPUT_ERROR;
            goto cleanup;
        }
    } else{
        ret = INPUT_ERROR;
        goto cleanup;
    }

    fpr = fopen(list, "r");
    if (fpr == NULL){
        ret = IO_ERROR;
        goto cleanup;
    }

    is_found = malloc((size_t)nkeys * sizeof(int));
    if (is_found == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
    }
    for (i = 0; i < nkeys; i = i + 1){
        is_found[i] = false;
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

    count = 0;
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
            line_changed = false;
            for (i = 0; i < nkeys; i = i + 1){
                if (is_found[i] == false && strcmp(work_key, keys[i]) == 0){
                    result = parse_meta(work_meta, &work_datetime, &work_ntags, &work_curr_tags);
                    if (result != 0){
                        unlink(tmpfile);
                        if (result == LIST_FORMAT_ERROR){
                            ret = LIST_FORMAT_ERROR;
                        } else if (result == MALLOC_ERROR){
                            ret = MALLOC_ERROR;
                        } else{
                            ret = UNKNOWN_ERROR;
                        }
                        goto cleanup;
                    }
                    if (imode == mode_tag){
                        result = tags_add(&work_ntags, &work_new_tags, work_curr_tags, info);
                        if (result != 0){
                            unlink(tmpfile);
                            if (result == MALLOC_ERROR){
                                ret = MALLOC_ERROR;
                            } else if (result == INPUT_ERROR){
                                ret = UNKNOWN_ERROR;
                            } else{
                                ret = UNKNOWN_ERROR;
                            }
                            goto cleanup;
                        }
                    } else if (imode == mode_utag){
                        result = tags_del(&work_ntags, &work_new_tags, work_curr_tags, info);
                        if (result != 0){
                            unlink(tmpfile);
                            if (result == MALLOC_ERROR){
                                ret = MALLOC_ERROR;
                            } else if (result == INPUT_ERROR){
                                ret = UNKNOWN_ERROR;
                            } else{
                                ret = UNKNOWN_ERROR;
                            }
                            goto cleanup;
                        }
                    }

                    result = make_meta(&work_new_meta, work_datetime, work_ntags, work_new_tags);
                    if (result != 0){
                        unlink(tmpfile);
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
                        .key   = work_key,
                        .file  = work_file,
                        .meta  = work_new_meta,
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

                    is_found[i]  = true;
                    line_changed = true;
                    changed      = true;
                    count        = count + 1;
                    XFREE(work_curr_tags);
                    XFREE(work_new_tags);
                    XFREE(work_new_meta);
                    break;
                }
            }
            if (line_changed == false){
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
        }
        XFREE(line);
    }

    if (changed == false){
        unlink(tmpfile);
        ret = KEY_NOT_FOUND;
        goto cleanup;
    }

    if (ferror(fpr)){
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

    goto cleanup;


cleanup:
    if (xfclose(&fpr)){
        if (ret == 0){
            ret = IO_ERROR;
        }
    }
    if (xfclose(&fpw)){
        if (ret == 0){
            ret = IO_ERROR;
        }
    }

    free(line);
    free(is_found);
    free(tmpfile);
    free(work_curr_tags);
    free(work_new_tags);
    free(work_new_meta);

    return ret;
}



