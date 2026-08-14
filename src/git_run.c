
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "globals.h"
#include "git_run.h"


// return FORK_ERROR if the operating system cannot create a child process
// return IO_ERROR if the specified working directory cannot be entered
// return EXECVP_ERROR if the git executable cannot be found or executed
// return WAIT_ERROR if the child process's termination status cannot be obtained
// return CHILD_ERROR if git does not complete successfully
// return 0 otherwise
int git_run(const char* dir, char* const* git_cmmd){
    pid_t pid;
    int   stat;
    int   exit_stat;

    pid = fork();

    if (pid == 0){
        if (chdir(dir) != 0){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
            _exit(IO_ERROR);
        }

        execvp("git", git_cmmd);
        fprintf(stderr, "%s: git: %s\n", PACKAGE_NAME, strerror(errno));
        _exit(EXECVP_ERROR);
    } else if (pid < 0){
        fprintf(stderr, "%s: fork: %s\n", PACKAGE_NAME, strerror(errno));
        return FORK_ERROR;
    }

    while (waitpid(pid, &stat, 0) < 0) {
        if (errno == EINTR){
            continue;
        }

        fprintf(stderr, "%s: waitpid: %s\n", PACKAGE_NAME, strerror(errno));
        return WAIT_ERROR;
    }

    if (!WIFEXITED(stat)){
        fprintf(stderr, "%s: git: terminated\n", PACKAGE_NAME);
        return CHILD_ERROR;
    }

    exit_stat = WEXITSTATUS(stat);

    if (exit_stat == 0){
        return 0;
    }

    if (exit_stat == IO_ERROR){
        return exit_stat;
    }

    if (exit_stat == EXECVP_ERROR){
        return exit_stat;
    }

    fprintf(stderr, "%s: git: exited with status %d\n", PACKAGE_NAME, exit_stat);
    return CHILD_ERROR;
}

