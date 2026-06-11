
#include <stdio.h>
#include <strings.h>
#include <time.h>

void get_datetime(char* datetime){
    time_t now;
    struct tm* lt;

    now = time(NULL);
    lt  = localtime(&now);
    sprintf(datetime, "%04d-%02d-%02d-%02d:%02d:%02d", lt->tm_year+1900,
                                                       lt->tm_mon +1   ,
                                                       lt->tm_mday     ,
                                                       lt->tm_hour     ,
                                                       lt->tm_min      ,
                                                       lt->tm_sec      );
}


