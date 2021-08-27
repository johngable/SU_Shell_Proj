#ifndef ENVIRON_H
#define ENVIRON_H

#include "list.h"

/**
 * @brief An environment variable
 * 
 * @param name char* name of the environment variable (ex. NAME)
 * @param contents char* the convents of envp[i]
 * @param list_head list, part of a list 
 * 
 */
struct environment {
    char *name; 
    char *contents; 
    struct list_head list; 
};

int set_env(struct list_head *list, char *name, char *value); 
int unset_env(struct list_head *list, char *name); 
void show_env(char **envp); 
void display_env_list(struct list_head *list); 
void display_env_array(char **envp); 
char ** make_env_array(struct list_head *list); 
void make_env_list(struct list_head *list, char **envp);  
void free_env_array(char **envp, int len); 
char * get_env(struct list_head *list, char *name); 
char * get_env_value(struct list_head *list, char *name);
void clear_list_env(struct list_head *list); 

#endif