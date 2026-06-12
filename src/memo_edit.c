

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


void get_command(char* editor, int editor_options_num, char** editor_options, int file_num, char** file, char** command){
    int i;
    int first_idx;

    command[0] = editor;
    #ifdef DEBUG
    printf("command[0] = %s\n", command[0]);
    #endif

    first_idx = 1;
    for (i = 0; i < editor_options_num; i = i + 1){
        command[i+first_idx] = editor_options[i];
        #ifdef DEBUG
        printf("command[%d] = %s\n", i+first_idx, command[i+first_idx]);
        #endif
    }

    first_idx = 1+editor_options_num;
    for (i = 0; i < file_num; i = i + 1){
        command[i+first_idx] = file[i];
        #ifdef DEBUG
        printf("command[%d] = %s\n", i+first_idx, command[i+first_idx]);
        #endif
    }

    command[1+editor_options_num+file_num] = NULL;
}


int memo_edit(char* dir, char* editor, int editor_options_num, char** editor_options, int file_num, char** file){
    pid_t pid;
    char** command;

    pid = fork();
    if (pid == 0){
        if (chdir(dir) != 0){
            perror(editor);
            _exit(1);
        }

        command = malloc((1+editor_options_num+file_num+1) * sizeof(char*));   // $(editor) $(editor_option) $(file) NULL
        get_command(editor, editor_options_num, editor_options, file_num, file, command);

        execvp(editor, command);
        perror(editor);
        _exit(1);
    } else if (pid < 0){
        perror("fork");
        return -1;
    }

    wait(NULL);
    return 0;
}


