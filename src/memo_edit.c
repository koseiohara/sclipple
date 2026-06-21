

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "globals.h"
#include "names.h"
#include "edit_list.h"


void get_command(char* editor, const int editor_options_num, char* const* editor_options, const int file_num, char* file[], char** command){
    int i;
    int base_idx;

    command[0] = editor;
    #ifdef DEBUG
    printf("command[0] = %s\n", command[0]);
    #endif

    base_idx = 1;
    for (i = 0; i < editor_options_num; i = i + 1){
        command[i+base_idx] = editor_options[i];
        #ifdef DEBUG
        printf("command[%d] = %s: original=editor_options[%d]\n", i+base_idx, command[i+base_idx], i);
        #endif
    }

    base_idx = 1 + editor_options_num;
    for (i = 0; i < file_num; i = i + 1){
        // #ifdef DEBUG
        // printf("file[%d] = %s\n", i, file[i]);
        // #endif
        command[i+base_idx] = file[i];
        #ifdef DEBUG
        printf("command[%d] = %s: original=file[%d]\n", i+base_idx, command[i+base_idx], i);
        #endif
    }

    command[1+editor_options_num+file_num] = NULL;
    #ifdef DEBUG
    printf("command[%d] set null manually\n", 1+editor_options_num+file_num);
    #endif
}


// return IO_ERROR if failed to open list file
// return LIST_FORMAT_ERROR if list file is broken
// return KEY_NOT_FOUND if keyword is not found in the list file
// return PROCESS_ERROR if failed to make a child process
// return 0 otherwise
int memo_edit(const char* list, const char* dir, char* editor, const int editor_options_num, char* const* editor_options, const int flag_num, char** flags){
    struct stat st;
    pid_t pid;
    FILE* fp;
    char** command = NULL;
    char** files;
    int i;
    int j;
    int result;

    result = path_status(list, &st);
    if (result != PATH_EXIST){
        if (result == PATH_NOT_EXIST){
            fprintf(stderr, "%s: No notes have been added\n", PACKAGE_NAME);
        } else if (result == ACCESS_FAILED_ERROR){
            fprintf(stderr, "%s: Failed to access list file\n", PACKAGE_NAME);
        }
        return IO_ERROR;
    } 

    files = malloc(flag_num * sizeof(char*));
    for (j = 0; j < flag_num; j = j + 1){
        files[j] = NULL;
    }

    fp = fopen(list, "r");
    if (fp == NULL){
        perror(list);
        return IO_ERROR;
    }
    for (i = 0; i < flag_num; i = i + 1){
        result = read_list_by_key(fp, flags[i], 2, &files[i]);
        if (result < 0){
            fprintf(stderr, "%s: list file is broken\n", PACKAGE_NAME);
            for (j = 0; j < flag_num; j = j + 1){
                free(files[j]);
            }
            free(files);
            fclose(fp);
            return LIST_FORMAT_ERROR;
        } else if (result == KEY_NOT_FOUND){
            fprintf(stderr, "%s: No such key: '%s'\nRun '%s add %s'\n", PACKAGE_NAME, flags[i], PACKAGE_NAME, flags[i]);
            for (j = 0; j < flag_num; j = j + 1){
                free(files[j]);
            }
            free(files);
            fclose(fp);
            return KEY_NOT_FOUND;
        }
        #ifdef DEBUG
        printf("Checked existence of %s\n", files[i]);
        #endif
    }
    fclose(fp);

    pid = fork();
    if (pid == 0){
        if (chdir(dir) != 0){
            perror(editor);
            for (j = 0; j < flag_num; j = j + 1){
                free(files[j]);
            }
            free(files);
            _exit(1);
        }

        command = malloc((1+editor_options_num+flag_num+1) * sizeof(char*));   // $(editor) $(editor_option) $(file) NULL
        if (command == NULL){
            perror("malloc");
            // free(command);
            for (j = 0; j < flag_num; j = j + 1){
                free(files[j]);
            }
            free(files);
            _exit(1);
        }
        get_command(editor, editor_options_num, editor_options, flag_num, files, command);

        execvp(editor, command);

        // if execvp() successed, the following processes never be executed
        perror(editor);

        free(command);
        for (j = 0; j < flag_num; j = j + 1){
            free(files[j]);
        }
        free(files);
        _exit(1);
    } else if (pid < 0){
        perror("fork");
        for (j = 0; j < flag_num; j = j + 1){
            free(files[j]);
        }
        free(files);
        return PROCESS_ERROR;
    }

    wait(NULL);

    for (j = 0; j < flag_num; j = j + 1){
        free(files[j]);
    }
    free(files);

    return 0;
}


