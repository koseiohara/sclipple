#ifndef HELP_H
#define HELP_H

void show_help_add(char* subdir, char* list);
void show_help_rm(void);
void show_help_mv(void);
void show_help_ls(void);
void show_help_search(void);
void show_help_show(void);
void show_help_edit(char* rc);
void show_help_git(void);
void show_help_config(char* rc);
void show_help_all(char* dir, char* subdir, char* list, char* rc);
void show_help_command(const char* command, char* dir, char* subdir, char* list, char* rc);
void show_help(char* dir, char* subdir, char* list, char* rc);

#endif
