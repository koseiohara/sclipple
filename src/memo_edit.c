

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


int memo_edit(const char* list, const char* dir, char* editor, const int editor_options_num, char* const* editor_options, const int flag_num, char** flags){
    struct stat st;
    pid_t pid;
    char** command = NULL;
    char*  files[flag_num];
    int i;
    int j;
    int result;

    result = path_status(list, &st);
    if (result != 1){
        if (result == 0){
            fprintf(stderr, "%s Error: No notes have been added\n", PROGRAM);
        } else if (result == -1){
            fprintf(stderr, "%s IO Error: Failed to access list file\n", PROGRAM);
        }
        return -1;
    } 

    for (j = 0; j < flag_num; j = j + 1){
        files[j] = NULL;
    }

    for (i = 0; i < flag_num; i = i + 1){
        result = get_filename_by_key(list, flags[i], files[i]);
        #ifdef DEBUG
        printf("Checked existence of %s\n", files[i]);
        #endif
        if (result == -1){
            fprintf(stderr, "%s Error: Invalid keyword. '%s' does not exist\n", PROGRAM, flags[i]);

            for (j = 0; j < flag_num; j = j + 1){
                free(flags[j]);
            }
            return -1;
        }
    }

    pid = fork();
    if (pid == 0){
        if (chdir(dir) != 0){
            perror(editor);
            for (j = 0; j < flag_num; j = j + 1){
                free(flags[j]);
            }
            _exit(1);
        }

        command = malloc((1+editor_options_num+flag_num+1) * sizeof(char*));   // $(editor) $(editor_option) $(file) NULL
        if (command == NULL){
            perror("malloc");
            for (j = 0; j < flag_num; j = j + 1){
                free(flags[j]);
            }
            _exit(1);
        }
        get_command(editor, editor_options_num, editor_options, flag_num, files, command);

        execvp(editor, command);
        perror(editor);
        for (j = 0; j < flag_num; j = j + 1){
            free(flags[j]);
        }
        _exit(1);
    } else if (pid < 0){
        perror("fork");
        for (j = 0; j < flag_num; j = j + 1){
            free(flags[j]);
        }
        return -1;
    }

    wait(NULL);

    for (j = 0; j < flag_num; j = j + 1){
        free(flags[j]);
    }

    return 0;
}


