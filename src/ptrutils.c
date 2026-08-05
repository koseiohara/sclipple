
#include <stdio.h>


int xfclose(FILE** fp){
    int ret;

    if (fp == NULL || *fp == NULL){
        return 0;
    }

    ret = fclose(*fp);
    *fp = NULL;
    return ret;
}



