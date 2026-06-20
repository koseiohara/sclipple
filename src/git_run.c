
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "globals.h"


// return PROCESS_ERROR if fork() failed
// return 0 otherwise
int git_run(const char* dir, char* const* git_cmmd){
    pid_t pid;

    pid = fork();

    if (pid == 0){
        if (chdir(dir) != 0){
            // perror("git error");
            fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
            _exit(1);
        }

        execvp("git", git_cmmd);
        perror("git error");
        _exit(1);
    } else if (pid < 0){
        perror("fork");
        return PROCESS_ERROR;
    }

    wait(NULL);
    return 0;
}

