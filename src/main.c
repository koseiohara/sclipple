
// #include <stdlib.h>
#include "git_run.h"

int main(int argc, char** argv){
    char dir[] = "/home/kosei/CLib/build/note-taker";
    char editor[] = "vim -p";
    git_run(dir, &argv[1]);
    return 0;
}

