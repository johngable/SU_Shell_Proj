/**
 * @file internal.c
 * @author Hannah Moats 
 * @brief Handle Internal Commands
 * @date 2021-03-22
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "datastructures.h"
#include "list.h"
#include "error.h"
#include "environ.h"

#define BUFFER_SIZE 4096

// Struct for internal commands, used to create a table of commands 
typedef struct internal {
  const char *name; 
  int (*handler)(struct subcommand *subcommand, struct list_head *list_env); 
} internal_t;  

/**
 * @brief If an error occurred then an error will be printed to the screen
 * with the name of the internal command that caused the error. 
 * @author Hannah Moats
 * 
 * @param status The value returned by a function that is being checked 
 * @param command The command that caused the error 
 */
static void check_status(int status, char *command) {
  if (status == -1) {
    fprintf(stderr, ERROR_INVALID_CMD, command); 
  }
}

/**
 * @brief Returns the number of arguments that are in the subcommand. An argument 
 * refers to the number of elements in the exec_args array, minus the NULL character
 * @author Hannah Moats
 * 
 * @param subcommand The subcommand that is being evaluated. 
 * @return int The number of args of the subcommand, not counting the NULL
 */
static int get_num_args(struct subcommand *subcommand) {
  int count = 0; 
  while (subcommand->exec_args[count] != NULL) {
    count++; 
  }
  return count; 
}

/**
 * @brief Get the first parsed argument that was entered on the command line.
 * This argument should be the name of the internal command that is being called.
 * @author Hannah Moats
 * 
 * @param subcommand The parsed command written on the command line
 * @return char* The name of the internal command (first argument)
 */
static char * get_internal_command(struct subcommand *subcommand) {
  return subcommand->exec_args[0]; 
}

/**
 * @brief Get the second parsed argument that was entered on the command line. 
 * This argument should be either a path name, or a name of an environment variable. 
 * @author Hannah Moats
 * 
 * @param subcommand The parsed command written on the command line
 * @return char* The name of an environment variable or path (second argument)
 */
static char * get_second_argument(struct subcommand *subcommand) {
  return subcommand->exec_args[1]; 
}

/**
 * @brief Get the third parsed argument that was entered on the command line. 
 * This argument should be a value used to set an environment variable. 
 * @author Hannah Moats
 * 
  * @param subcommand The parsed command written on the command line
 * @return char* The value entered by the user (third argument) 
 */
static char * get_third_argument(struct subcommand *subcommand) {
  return subcommand->exec_args[2]; 
}

/**
 * @brief Handles the setenv internal command. The set environment takes a name 
 * and sets the given value to that name. 
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline
 * @return int If an error occured, output is -1 else output is 0
 */
static int handle_setenv(struct subcommand *subcommand, struct list_head *list_env) {
  //checks that the setenv command is valid
  int num_args = get_num_args(subcommand); 
  if (num_args != 3) { //plus 1 since we store a NULL at the end
    fprintf(stderr, ERROR_SETENV_ARG); 
    return -1; 
  }

  //if the command is value set the apropiate environment variable 
  char *name = get_second_argument(subcommand); 
  char *value = get_third_argument(subcommand); 
  set_env(list_env, name, value); 
  int status = setenv(name, value, 1); 
  return 0; 
}

/**
 * @brief Handles the getenv internal command. The getenv command is used to 
 * either get a specified environment, given by a name. Or to print the entire environment. 
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline 
 * @return int If an error occured, output is -1 else output is 0
 */
static int handle_getenv(struct subcommand *subcommand, struct list_head *list_env) {
  int num_args = get_num_args(subcommand); 
  if (num_args == 1) { //subcommand: getenv
    display_env_list(list_env); 
  } else if (num_args == 2) { //subcommmand: getenv $NAME
    char *name = get_second_argument(subcommand); 
    char *env_list = get_env(list_env, name); 
    char *env = getenv(name); 

    //error check
    if (env_list == NULL) {
      fprintf(stderr, ERROR_GETENV_INVALID, name); 
      return -1; 
    }

    printf("%s\n", env_list); 
  } else { //error: there ere not enough arguments or too many 
    fprintf(stderr, ERROR_GETENV_ARG);
    return -1; 
  }
  return 0;
}

/**
 * @brief Handles the unsetenv command. The unsetenv command is used to delete the 
 * given name from the environment. 
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline
 * @return int If an error occured, output is -1 else output is 0
 */
static int handle_unsetenv(struct subcommand *subcommand, struct list_head *list_env) {
  int num_args = get_num_args(subcommand); 
  if (num_args != 2) { //plus 1 since we store a NULL at the end
    fprintf(stderr, ERROR_UNSETENV_ARG); 
    return -1; 
  }
  char *name = get_second_argument(subcommand); 
  int status = unset_env(list_env, name); 
  status = unsetenv(name); 
  
  return 0;
}

/**
 * @brief Handles the change directory internal command. The change directory command
 * either sets the current directory to the HOME directory, or changes to a specific 
 * directory given a path. 
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline
 * @return int If an error occured, output is -1 else output is 0
 */
static int handle_cd(struct subcommand *subcommand, struct list_head *list_env) {
  int num_args = get_num_args(subcommand); 
  if (num_args == 1) { //subcommand: cd
    char *home = getenv("HOME"); //get home env variable 
    int status = chdir(home); 

    //check for error
    if (status == -1) {
      fprintf(stderr, ERROR_CD_NOHOME); 
      return -1; 
    }
  } else if (num_args == 2) { //subcommand: cd pathname
    char *path = get_second_argument(subcommand); 
    int status = chdir(path); 

    check_status(status, "cd"); 

  } else { //too many args 
    fprintf(stderr, ERROR_CD_ARG); 
    return -1;    
  }

    return 0;

}

/**
 * @brief Handles the print working directory internal command. The print working direcotry
 * command calls getcwd() system call and is used to print the current directory
 * that the user is in. 
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline
 * @return int If an error occured, output is -1 else output is 0
 */
static int handle_pwd(struct subcommand *subcommand, struct list_head *list_env) {
  //#define ERROR_PWD_ARG "Error - pwd takes no arguments\n"
  int num_args = get_num_args(subcommand); 
  if (num_args != 1) { //subcommand is NOT: pwd
    fprintf(stderr, ERROR_PWD_ARG); 
    return -1; 
  } 

  char buf[BUFFER_SIZE]; 
  char *status = getcwd(buf, sizeof(buf)); 
  printf("%s\n", buf); 
  memset(buf, 0, BUFFER_SIZE); 

  if (status == NULL) {
    check_status(-1, "pwd"); 
  }

  return 0;
}
 
/**
 * @brief Handles the exit internal command. Not only does the command exit the
 * shell, but it also frees any malloced data to ensure there are no memory leaks. 
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline
 * @return int Returns a code to be processed by sush to clear list_command and exit. 
 */
static int handle_exit(struct subcommand *subcommand, struct list_head *list_env) {
  int num_args = get_num_args(subcommand); 
  if (num_args != 1) {
    fprintf(stderr, ERROR_EXIT_ARG); 
    return -1;
  }
  return 6; 
}

// Declaring a table of internal commands that will be crossreferenced to when processing a command 
internal_t internal_cmds[] = {
  { .name = "setenv" , .handler = handle_setenv }, 
  { .name = "getenv" , .handler = handle_getenv },
  { .name = "unsetenv" , .handler = handle_unsetenv },
  { .name = "cd", .handler = handle_cd }, 
  { .name = "pwd", .handler = handle_pwd }, 
  { .name = "exit", .handler = handle_exit}, 
  0
};

/**
 * @brief Given the command on the command line, this function determines
 * what command needs to be handled and calls the respective function.
 * @author Hannah Moats
 * 
 * @param subcommand A parsed command from the commandline
 * @param list_env list_head List of environment variables
 * @return int If an error occured, output is -1 else output is 0
 */
int handle_internal(struct list_head *commands, struct list_head *list_env) {
  int i = 0; 
  struct subcommand *entry;
  entry = list_entry(commands->next, struct subcommand, list); 

  while(internal_cmds[i].name != 0) {
    char *command_name = get_internal_command(entry); 
    if (!strcmp(internal_cmds[i].name, command_name)) {
      return internal_cmds[i].handler(entry, list_env); 
    }
    i++;
  } 
  return 1; 
}
