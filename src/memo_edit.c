
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "names.h"
#include "edit_list.h"


void get_command(char* editor, const int file_num, char* file[], char** command){
    int i;
    int corrector;
    int base_idx;

    command[0] = "sh";
    command[1] = "-c";
    command[2] = editor;
    command[3] = "sh";

    base_idx  = 4;
    corrector = 0;
    for (i = 0; i < file_num; i = i + 1){
        if (file[i] != NULL){
            command[i+base_idx-corrector] = file[i];
        } else{
            corrector = corrector + 1;
        }
    }

    command[4+file_num-corrector] = NULL;
}


// return IO_ERROR if the list file or working directory cannot be accessed
// return MALLOC_ERROR if memory required for the command cannot be allocated
// return LIST_FORMAT_ERROR if the list file contains an invalid entry
// return KEY_NOT_FOUND if a requested key does not exist in the list file
// return INPUT_ERROR if an invalid column is passed to the list reader
// return UNKNOWN_ERROR if the list reader returns an unexpected error
// return FORK_ERROR if the operating system cannot create a child process
// return WAIT_ERROR if the child process's termination status cannot be obtained
// return EXECVP_ERROR if the shell executable cannot be found or executed
// return CHILD_ERROR if the editor command does not complete successfully
// return 0 otherwise
int memo_edit(const char* list, const char* dir, char* editor, const int flag_num, char** flags){
    struct stat st;
    pid_t  pid;
    FILE*  fp = NULL;
    char** command    = NULL;
    char** files      = NULL;
    char*  tmp_editor = NULL;
    int i;
    int j;
    int command_len;
    int result;
    int ret = 0;
    int stat;
    int exit_stat;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        }
        ret = IO_ERROR;
        goto cleanup;
    } 

    files = malloc(flag_num * sizeof(char*));
    if (files == NULL){
        fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
        ret = MALLOC_ERROR;
        goto cleanup;
    }

    for (j = 0; j < flag_num; j = j + 1){
        files[j] = NULL;
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
        ret = IO_ERROR;
        goto cleanup;
    }
    for (i = 0; i < flag_num; i = i + 1){
        rewind(fp);
        result = read_list_by_key(fp, flags[i], 2, &files[i]);
        if (result != 0){
            if (result == KEY_NOT_FOUND){
                fprintf(stderr, "%s: No such key: '%s'\nRun '%s add %s'\n", PACKAGE_NAME, flags[i], PACKAGE_NAME, flags[i]);
                ret = KEY_NOT_FOUND;
                files[i] = NULL;
                // goto cleanup;
            } else if (result == LIST_FORMAT_ERROR){
                fprintf(stderr, "%s: List file is broken\n", PACKAGE_NAME);
                ret = LIST_FORMAT_ERROR;
                goto cleanup;
            } else if (result == MALLOC_ERROR){
                fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
                ret = MALLOC_ERROR;
                goto cleanup;
            } else if (result == INPUT_ERROR){
                fprintf(stderr, "%s: Bug: Invalid col\n", PACKAGE_NAME);
                ret = INPUT_ERROR;
                goto cleanup;
            } else if (result == IO_ERROR){
                fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, list, strerror(errno));
                ret = IO_ERROR;
                goto cleanup;
            } else{
                fprintf(stderr, "%s: Unknown error\n", PACKAGE_NAME);
                ret = UNKNOWN_ERROR;
                goto cleanup;
            }
        }
        #ifdef DEBUG
        printf("Checked existence of %s\n", files[i]);
        #endif
    }
    xfclose(&fp);

    pid = fork();
    if (pid == 0){
        if (chdir(dir) != 0){
            fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, editor, strerror(errno));
            _exit(IO_ERROR);
        }

        result = asprintf(&tmp_editor, "%s \"$@\"", editor);
        if (result < 0){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            _exit(MALLOC_ERROR);
        }
        command_len = 4+flag_num+1;
        command = malloc(command_len * sizeof(char*));   // sh -c "rc input" sh file1 file2 ... NULL
        if (command == NULL){
            fprintf(stderr, "%s: %s\n", PACKAGE_NAME, strerror(errno));
            _exit(MALLOC_ERROR);
        }
        get_command(tmp_editor, flag_num, files, command);
        if (command[4] == NULL){
            fprintf(stderr, "%s: No available key\n", PACKAGE_NAME);
            _exit(KEY_NOT_FOUND);
        }

        // If this shell command successfully executed, the following lines never be executed
        execvp("sh", command);
        fprintf(stderr, "%s: %s: %s\n", PACKAGE_NAME, editor, strerror(errno));
        _exit(EXECVP_ERROR);
    } else if (pid < 0){
        fprintf(stderr, "%s: fork: %s\n", PACKAGE_NAME, strerror(errno));
        ret = FORK_ERROR;
        goto cleanup;
    }

    while (waitpid(pid, &stat, 0) < 0) {
        if (errno == EINTR){
            continue;
        }

        fprintf(stderr, "%s: waitpid: %s\n", PACKAGE_NAME, strerror(errno));
        ret = WAIT_ERROR;
        goto cleanup;
    }

    if (!WIFEXITED(stat)){
        fprintf(stderr, "%s: %s: terminated\n", PACKAGE_NAME, editor);
        ret = CHILD_ERROR;
        goto cleanup;
    }

    exit_stat = WEXITSTATUS(stat);

    if (exit_stat == 0){
        goto cleanup;
    }

    if (exit_stat == KEY_NOT_FOUND){
        ret = exit_stat;
        goto cleanup;
    }

    if (exit_stat == IO_ERROR){
        ret = exit_stat;
        goto cleanup;
    }

    if (exit_stat == MALLOC_ERROR){
        ret = exit_stat;
        goto cleanup;
    }

    if (exit_stat == EXECVP_ERROR){
        ret = exit_stat;
        goto cleanup;
    }

    fprintf(stderr, "%s: %s: exited with status %d\n", PACKAGE_NAME, editor, exit_stat);
    ret = CHILD_ERROR;
    goto cleanup;


cleanup:
    xfclose(&fp);
    if (files != NULL){
        for (j = 0; j < flag_num; j = j + 1){
            free(files[j]);
        }
    }
    free(command);
    free(files);
    free(tmp_editor);

    return ret;
}


