
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "globals.h"
#include "ptrutils.h"
#include "names.h"
#include "edit_list.h"


void get_command(char* editor, const int file_num, char* file[], char** command){
    int i;
    int base_idx;

    command[0] = "sh";
    command[1] = "-c";
    command[2] = editor;
    command[3] = "sh";

    base_idx = 4;
    for (i = 0; i < file_num; i = i + 1){
        command[i+base_idx] = file[i];
    }

    command[4+file_num] = NULL;

    // i = 0;
    // while(command[i] != NULL){
    //     printf("%s\n", command[i]);
    //     i = i + 1;
    // }
    // printf("%s\n", command[i]);
    // exit(1);
}


// return IO_ERROR if failed to open list file
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if keyword is not found in the list file
// return PROCESS_ERROR if failed to make a child process
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
    int ret;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        }
        ret = IO_ERROR;
        goto cleanup;
        // return IO_ERROR;
    } 

    files = malloc(flag_num * sizeof(char*));
    if (files == NULL){
        ret = MALLOC_ERROR;
        goto cleanup;
        // return MALLOC_ERROR;
    }

    for (j = 0; j < flag_num; j = j + 1){
        files[j] = NULL;
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        ret = IO_ERROR;
        goto cleanup;
        // return IO_ERROR;
    }
    for (i = 0; i < flag_num; i = i + 1){
        rewind(fp);
        result = read_list_by_key(fp, flags[i], 2, &files[i]);
        if (result < 0){
            fprintf(stderr, "%s: list file is broken\n", PACKAGE_NAME);
            ret = LIST_FORMAT_ERROR;
            goto cleanup;
            // for (j = 0; j < flag_num; j = j + 1){
            //     free(files[j]);
            // }
            // free(files);
            // fclose(fp);
            // return LIST_FORMAT_ERROR;
        } else if (result == KEY_NOT_FOUND){
            fprintf(stderr, "%s: No such key: '%s'\nRun '%s add %s'\n", PACKAGE_NAME, flags[i], PACKAGE_NAME, flags[i]);
            ret = KEY_NOT_FOUND;
            goto cleanup;
            // for (j = 0; j < flag_num; j = j + 1){
            //     free(files[j]);
            // }
            // free(files);
            // fclose(fp);
            // return KEY_NOT_FOUND;
        }
        #ifdef DEBUG
        printf("Checked existence of %s\n", files[i]);
        #endif
    }
    XFCLOSE(fp);

    pid = fork();
    if (pid == 0){
        if (chdir(dir) != 0){
            perror(editor);
            // for (j = 0; j < flag_num; j = j + 1){
            //     free(files[j]);
            // }
            // free(files);
            _exit(1);
        }

        result = asprintf(&tmp_editor, "%s \"$@\"", editor);
        if (result < 0){
            perror("asprintf");
            // for (j = 0; j < flag_num; j = j + 1){
            //     free(files[j]);
            // }
            // free(files);
            _exit(1);
        }
        command_len = 4+flag_num+1;
        command = malloc(command_len * sizeof(char*));   // sh -c "rc input" sh file1 file2 ... NULL
        if (command == NULL){
            perror("malloc");
            // free(command);
            // for (j = 0; j < flag_num; j = j + 1){
            //     free(files[j]);
            // }
            // free(files);
            // free(tmp_editor);
            _exit(1);
        }
        get_command(tmp_editor, flag_num, files, command);

        // If this shell command successfully executed, the following lines never be executed
        execvp("sh", command);

        // if execvp() successed, the following processes never be executed
        perror(editor);

        // free(command);
        // for (j = 0; j < flag_num; j = j + 1){
        //     free(files[j]);
        // }
        // free(files);
        // free(tmp_editor);
        _exit(1);
    } else if (pid < 0){
        perror("fork");
        ret = PROCESS_ERROR;
        goto cleanup;
        // for (j = 0; j < flag_num; j = j + 1){
        //     free(files[j]);
        // }
        // free(files);
        // return PROCESS_ERROR;
    }

    wait(NULL);

    ret = 0;
    goto cleanup;
    // for (j = 0; j < flag_num; j = j + 1){
    //     free(files[j]);
    // }
    // free(files);

    // return 0;


cleanup:
    XFCLOSE(fp);
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


