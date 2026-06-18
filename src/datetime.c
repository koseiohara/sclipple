
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

void get_datetime(struct tm* clock, char delim, char* datetime){
    int len;
    len = 20;

    datetime = malloc(len * sizeof(char));

    if (delim == '\0'){
        snprintf(datetime, len, "%04d/%02d/%02d %02d:%02d:%02d", clock->tm_year+1900,
                                                                 clock->tm_mon +1   ,
                                                                 clock->tm_mday     ,
                                                                 clock->tm_hour     ,
                                                                 clock->tm_min      ,
                                                                 clock->tm_sec      );
    } else{
        snprintf(datetime, len, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d", clock->tm_year+1900,
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
}


