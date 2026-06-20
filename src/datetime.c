
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

#include "globals.h"


// return MALLOC_ERROR if asprintf errors
// return 0 otherwise
int get_datetime(struct tm* clock, char delim, char** datetime){
    int result;

    if (delim == '\0'){
        result = asprintf(datetime, "%04d/%02d/%02d %02d:%02d:%02d", clock->tm_year+1900,
                                                                     clock->tm_mon +1   ,
                                                                     clock->tm_mday     ,
                                                                     clock->tm_hour     ,
                                                                     clock->tm_min      ,
                                                                     clock->tm_sec      );
    } else{
        result = asprintf(datetime, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d", clock->tm_year+1900,
                                                                          delim              ,
                                                                          clock->tm_mon +1   ,
                                                                          delim              ,
                                                                          clock->tm_mday     ,
                                                                          delim              ,
                                                                          clock->tm_hour     ,
                                                                          delim              ,
                                                                          clock->tm_min      ,
                                                                          delim              ,
                                                                          clock->tm_sec      );
    }

    #ifdef DEBUG
    printf("<DEBUG> DATETIME = %s\n", datetime);
    #endif

    if (result < 0){
        perror("asprintf");
        return MALLOC_ERROR;
    } else{
        return 0;
    }
}


