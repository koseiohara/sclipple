
#include <stdio.h>
#include <strings.h>
#include <time.h>

void get_datetime(struct tm* clock, char delim, size_t datetime_len, char* datetime){
    // time_t now;
    // struct tm* lt;

    // now = time(NULL);
    // lt  = localtime(&now);
    snprintf(datetime, datetime_len, "%04d%c%02d%c%02d%c%02d%c%02d%c%02d", clock->tm_year+1900,
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
    #ifdef DEBUG
    printf("<DEBUG> DATETIME = %s\n", datetime);
    #endif
}


