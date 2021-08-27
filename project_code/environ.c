/**
 * @file environ.c
 * @author Hannah Moats
 * @brief Handles the shells internal environment. 
 * @version 0.1
 * @date 2021-03-25
 * 
 * @copyright Copyright (c) 2021
 */

// Includes
#include <stdio.h> // For IO (printf)
#include <string.h> // For handling strings
#include <stdlib.h> // For malloc, calloc, free, etc

#include "environ.h" // Header file

#define BUFFER_SIZE 4096 // Max size of a char*

/**
 * @brief Used by set_env to update the contents in the environment struct. 
 * 
 * @param env The environment struct that is updated. 
 * @param name The name of the environment variable being updated
 * @param value The value that is assigned to the name variable 
 * @param list The list of environment variables
 */
static void set_env_update_contents(struct environment *env, char *name, char *value, struct list_head *list) {
  int len = strlen(name) + strlen(value) + 2; // Length of strings and null terms 
  env->contents = calloc(len, sizeof(char)); // Allocate to fit new contents
  strcat(env->contents, name); // Copy new name
  strcat(env->contents, "="); // Copy equals sign
  strcat(env->contents, value); // Copy value
  return; 
}

/**
 * @brief Set an environment variable.
 * 
 * @param list list_head List of environment variables
 * @param name char* name of environment variable to change
 * @param value char* value to change environment variable to 
 * @return int Returns 0 upon success, returns -1 if unsuccessful/error ocurred
 */
int set_env(struct list_head *list, char *name, char *value) {
  struct environment *env; // Environment to search through
  struct list_head *curr; // Current node in list
  curr = list; // Set current

  // First, search through list_head to find if environment variable exists. 

  // Iterate through list until we reach the beginning again. 
  for (curr = curr->next; curr != list; curr = curr->next) {
    env = list_entry(curr, struct environment, list); // Update environment 
    // If the environment name matches the name we are searching for 
    if (strcmp(env->name, name) == 0) {
      // Update contents
      free(env->contents); // Free old contents
      set_env_update_contents(env, name, value, list); 
      return 0;  // Return 0 for success
    }
  }

  // If environment variable was not found in list of environment variables, add it!

  env = malloc(sizeof(struct environment)); // Allocate space for a new environment variable
  env->name = strdup(name); // Add name to environment variable
  set_env_update_contents(env, name, value, list); 
  list_add_tail(&env->list, list); // Add environment to tail


  return 0; // Return 0 for success
}

/**
 * @brief Unset an environment variable. 
 * 
 * @param list_env list_head list to remove environment variable from. 
 * @param name char* name of environment to remove.  
 * @return int Returns 0 upon success, returns -1 if unsuccessful/error ocurred
 */
int unset_env(struct list_head *list_env, char *name) {
    struct environment *entry; // Environment to search through
    struct list_head *curr; // Current node in list

    curr = list_env; // Update node 
    // Iterate through list until we reach the beginning again. 
    for (curr = curr->next; curr != list_env; curr = curr->next) {
        entry = list_entry(curr, struct environment, list); // Update environment 
        // If environment name matches name we are searching through 
        if (strcmp(entry->name, name) == 0) {
            list_del(&entry->list); // Delete node from list
            free(entry->name); // Free name 
            free(entry->contents); // Free contents
            free(entry); // Free node 
            return 0; // Return 0 for success
        }
    }
}

/**
 * @brief Returns the environment variable name. 
 * 
 * @param contents A string with an environment variable in it (NAME=...). 
 * @return char* The environment variable name. 
 */
static char * get_env_variable_name(char const *contents) {
  int len = strlen(contents) + 1; // Length of string + null term
  char *new_contents = malloc(len * sizeof(char)); // Allocate space for new contents
  strncpy(new_contents, contents, len); // Copy contents to new contents
  char *name; // New variable to hold name of environment variable 
  name = strtok(new_contents, "="); // Strtok at "=" to get name
  return name; // Return name
}

/**
 * @brief Get the environment variable value.
 * 
 * @param contents A string with the environment variable in in (NAME=...).
 * @return char* The value of the environment variable. 
 */
static char * get_env_variable_value(char *contents) {
  while (*contents != '=' && *contents != '\0') {
    contents++; 
  }
  contents++; 
  return contents; // Return value
}

/**
 * @brief Given a list head for the list that contains the environment variable it returns the 
 * whole environment variable. 
 * 
 * @param list list_head The list that contains the environment variables.
 * @param name char* The name of the environment variable searched for in the shell's internal environment.
 * @return char* The contents of the envirnment variable or NULL if the environment variable is not there 
 */
char * get_env(struct list_head *list, char *name) {
  struct environment *env; // Environnent to look through
  struct list_head *curr; // Current node

  curr = list->next; // Update node
  // Iterate through list until we reach the beginning again
  while (curr != list) {
    env = list_entry(curr, struct environment, list); // Update environment 
    curr = curr->next;
    // If the env name matches the name passed, return the contents
    if (!strcmp(env->name, name)) { 
      //return get_env_variable_value(env->contents); 
       return env->contents;
    }
  }
  return NULL; // If we did not find the environment in the list
}

/**
 * @brief Given a list head for the list that contains the environment variable it returns the 
 * environment variable value.
 * 
 * @param list list_head The list that contains the environment variables.
 * @param name char* The name of the environment variable searched for in the shell's internal environment.
 * @return char* The value of the envirnment variable or NULL if the environment variable is not there 
 */
char * get_env_value(struct list_head *list, char *name) {
  struct environment *env; // Environment to look through 
  struct list_head *curr; // Current node

  curr = list->next; // Update node
  // Iterate through list until we reach the beginning again
  while (curr != list) {
    env = list_entry(curr, struct environment, list); // Update environment 
    curr = curr->next; 
    // If the env name matches the name passed, return the contents
    if (!strcmp(env->name, name)) { 
      // Return the value of the variable
      return get_env_variable_value(env->contents); 
    }
  }
  return NULL; // If we didn't find the environment in the list
}

/**
 * @brief Frees the environment array. 
 * 
 * @param envp The array that is being freed. 
 * @param len The length of the array being freed. 
 */
void free_env_array(char **envp, int len) {
  // Iterate through environment array 
  for (int i = 0; i < len; i++) {
    free(envp[i]); // Free environment 
  }
  free(envp); // Free 2D array
}

/**
 * @brief Clear a list environment.
 * 
 * @param list list_head to free
 */
void clear_list_env(struct list_head *list) {
  struct environment *entry; // Environment to potentially clear
  // Iterate through list while it is not empty
  while (!list_empty(list)) {
    entry = list_entry(list->next, struct environment, list); // Update environment 
    list_del(&entry->list); // Delete entry from list
    free(entry->name); // Free name 
    free(entry->contents); // Free contents
    free(entry); // Free entry
  }
}

/**
 * @brief Take in the envp list and displays it on the screen.
 * 
 * @param list_envp The list_head of the list that is being displayed
 */
void display_env_list(struct list_head *envp_list) {
  struct list_head *start = envp_list->next; // Start at the first node after the head
  struct list_head *curr; // Tracks current node during traversal
  struct environment *entry; // Current nodes struct with contents

  // Iterate through list until we reach the beginning again
  for (curr = envp_list->next; curr->next != start; curr = curr->next)
  {
    entry = list_entry(curr, struct environment, list); // Update entry 
    printf("%s\n", entry->contents); // Print the contents of entry 
  }
}

/**
 * @brief Takes in the envp array and displays it on the screen. 
 * 
 * @param envp The environment variable array that is displayed. 
 */
void display_env_array(char **envp) {
  int i = 0; 
  // Iterate through envp until a null is reached
  while (envp[i] != NULL) {
    printf("%s\n", envp[i]); // Print
    i++; // Move to next
  }
}

/**
 * @brief Make the environment array. 
 * 
 * @param list list_head to make array from.
 * @return char** array of environment variables.
 */
char ** make_env_array(struct list_head *list) {
    struct environment *entry; // Current environment 
    struct list_head *curr; // Current node

    int list_len = getListLength(list); // Get size of list
    int i = 0; // Used to know which environment variable we are on.

    char **envp = calloc(list_len + 1, sizeof(char *)); 
    for (curr = list->next; curr != list; curr = curr->next) {
        entry = list_entry(curr, struct environment, list); // Update environment variable 
        envp[i] = strdup(entry->contents); // Copy its contents
        i++; // Go on to next environment 
    }
    envp[list_len] = NULL; // Set highest to null, it cannot be accessed. 

    return envp; // Return new environment array
}

/**
 * @brief Takes the array of environment variables from main and makes a linked list
 * 
 * @param list The list that the environment variables are added to 
 * @param envp The environment variable array that is being made into a list
 */
void make_env_list(struct list_head *list, char **envp) {
  int i = 0; // Used to keep track of which environment we are on
  // Iterate through 2D array until we reach null (end of 2D array)
  while (envp[i] != NULL) { 
    struct environment *env = malloc(sizeof(struct environment)); // Update environment
    env->contents = strdup(envp[i]); // Copy contents
    char *name = get_env_variable_name(env->contents); // Get name 
    env->name = name; // Add name to list
    list_add(&env->list, list); // Add to tail
    i++; // Move on to next node
  }
}