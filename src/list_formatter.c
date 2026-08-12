
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

#include "list_formatter.h"

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


// ntags is ignored if tags == NULL
//
// return INPUT_ERROR if one or more arguments are invalid
// return IO_ERROR if io failed
// return MALLOC_ERROR if malloc failed
// return 0 otherwise
int make_meta(char** meta, const char* datetime, const int ntags, char* const* tags){
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

    if (datetime == NULL){
        ret = INPUT_ERROR;
        goto cleanup;
    }

    if (ntags < 0 && tags != NULL){
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


