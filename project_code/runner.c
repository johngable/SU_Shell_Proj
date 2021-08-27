/**
 * @file runner.c
 * @author Isabella Boone 
 * @author John Gable
 * @author Hannah Moats
 * @brief Handle running user input and .sushrc file. 
 * @version 0.1
 * @date 2021-03-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h> // for i/o
#include <stdlib.h> // for memory allocation
#include <sys/stat.h> // for stat system call

// Imports from our files
#include "runner.h"

/**
 * @brief Clear a list of commands. 
 * 
 * @param list list_head to be cleared. 
 */
static void clear_list_command(struct list_head *list) {
  struct subcommand *entry; 
  while (!list_empty(list)) {
    entry = list_entry(list->next, struct subcommand, list);
    
    // Free 2D array exec_args
    int i = 0; // Count where we are in array
    while (entry->exec_args[i] != NULL) {
      free(entry->exec_args[i]); // Free array elements
      i++; 
    }
    // Free entry and everything inside it
    free(entry->exec_args);  
    free(entry->input); 
    free(entry->output); 
    list_del(&entry->list); 
    free(entry); 
  }
}

/**
 * @brief Free the commandline struct. 
 * 
 * @param cmdline commandline to be freed.
 */
static void free_commandline_struct(commandline cmdline) {
  // Free everything inside commandline
  for(int i=0;i<cmdline.num; i++){
    free(cmdline.subcommand[i]);
  }
  free(cmdline.subcommand); // Free commandline
}

/**
 * @brief Free everything on exit. 
 * 
 * @param list_commands list_head list of commands to be cleared. 
 * @param list_env list_head list of environment variables to be cleared. 
 * @param cmdline commandline to be cleared. 
 */
static void freeing_on_exit(struct list_head *list_commands, struct list_head *list_env, commandline cmdline) {
  clear_list_command(list_commands); // Clear command
  clear_list_env(list_env); // Clear environments
  free_commandline_struct(cmdline); // Free commandline
}


/**
 * @brief Checks to ensure that SUSHHOME environment variable has been set. 
 * 
 * @param list_env list_head to get SUSHHOME environment variable from. 
 * @return int if set, 1, else, 0. 
 */
static int sushhome_exists(struct list_head *list_env){
  char* sushhome = get_env(list_env, "SUSHHOME"); // Get environment variable
  if(sushhome != NULL){
    return 1; 
  }
  return 0;
}

/**
 * @brief Takes a command line and runs the command.
 * 
 * @param list_commands The list of comamnds, which is a list of subcommands
 * @param list_env The list_env that holds all the environment variables 
 * @param list_args The list of argumentst that are parsed from the command line
 * @param cmdline Struct which holds, unparsed subcommands, and the number of subcommands.
 * @param input The input buffer for fgets
 */
static void run_parser_executor_handler(struct list_head *list_commands, struct list_head *list_env, struct list_head *list_args, commandline cmdline, char *input) {


  int len = strlen(input); 
  
  if(input[len-1]=='\n'){
    input[len-1] = '\0';
  }
  
  cmdline.num = find_num_subcommands(input, len);
            
  //creates an array of pointers, in proportion to the number of subcommands
  cmdline.subcommand = malloc(cmdline.num *  sizeof(char *)); 
  copy_subcommands(input, cmdline.num, cmdline.subcommand);
  int valid_cmdline = parse_commandline(list_args, &cmdline, list_commands);

  if (valid_cmdline == 0) { //If there were no errors when parsing 
    //Checks if an internal command, if it is then it is run, else a normal command is run
    int internal_code = handle_internal(list_commands, list_env);
    if(internal_code == 1) { 
      char **new_envp = make_env_array(list_env); 
      run_command(cmdline.num, list_commands, new_envp);
      clear_list_env(list_env); 
      make_env_list(list_env, new_envp);
      int list_env_len = getListLength(list_env); 
      //free_env_array(new_envp, list_env_len);  
    } else if (internal_code == 6) {  //if internal code was to exit
      freeing_on_exit(list_commands, list_env, cmdline);
      exit(0);
    }
  }

  //Free what we no longer need
  free_commandline_struct(cmdline);   
  clear_list_command(list_commands); 
}

/**
 * @brief Retrieves the sushrc files path from the SUSHHOME directory if it exists.
 * 
 * @param list_env The list_env that holds all the environment variables 
 *
 * @return char* filename - The complete path for .sushrc (including the .sushrc in the path)
 */
static char *getsushrc(struct list_head *list_env )
{
  char *filename = calloc(1024, sizeof(char));

  if(sushhome_exists(list_env)){
    struct stat sb; //Keep track of information regarding the .sushrc file
    char* sushhome = get_env_value(list_env, "SUSHHOME"); //Get location to look for sushrc 
    strcpy(filename,sushhome);
    strcat(filename, "/.sushrc");
  } else {
    strcpy(filename,".sushrc");
  }  

  return filename;
}

/**
 * @brief Checks to see if SUSHHOME environment variable was set, if so executes the .sushrc file
 * 
 * @param list_commands The list of comamnds, which is a list of subcommands
 * @param list_env The list_env that holds all the environment variables 
 * @param list_args The list of argumentst that are parsed from the command line
 * @param cmdline Struct which holds, unparsed subcommands, and the number of subcommands.
 * @param input The input buffer for fgets
 */
void run_rc_file(struct list_head *list_commands, struct list_head *list_env, struct list_head *list_args, commandline cmdline, char *input) {
  char *fname = NULL;
  if(sushhome_exists(list_env)){
    struct stat sb; //Keep track of information regarding the .sushrc file
    
    fname = getsushrc(list_env);
    // printf("%s\n", fname);
    int stat_status = stat(fname, &sb);
    if ((sb.st_mode & S_IRUSR) && (sb.st_mode & S_IXUSR)) { //if true file is valid, read from file
      FILE *file = fopen(fname, "r");   //open .suhrc and read from it
      
      if (file == NULL) {
        // printf("This file was null\n");
        goto error;
      }
      //read from file and execute commands 
      while (fgets(input, INPUT_LENGTH-1, file)) {
        // printf("contents: %s\n", input);
        run_parser_executor_handler(list_commands, list_env, list_args, cmdline, input); 
      } 
      int flcose_status = fclose(file); 
    }
  } 


error:
  if (fname != NULL)
    free(fname);
  return;  
}

/**
 * @brief Checks to see if the PS1 variable has been set by rc/envp, if not default to ">"
 * 
 * @param list_env - Internal environment list from main
 */
void check_PS1(struct list_head *list_env) {
  //checks if PS1 is set, if it is then there is no need to set the environment
  if (get_env(list_env, "PS1") != NULL) {
    printf("%s", get_env_value(list_env, "PS1"));  
  } else {
    printf(">");
  } 
  fflush(stdout); 
}

/**
 * @brief Takes a command line from user input and runs it.
 * 
 * @param list_commands The list of comamnds, which is a list of subcommands
 * @param list_env The list_env that holds all the environment variables 
 * @param list_args The list of argumentst that are parsed from the command line
 * @param cmdline Struct which holds, unparsed subcommands, and the number of subcommands.
 * @param input The input buffer for fgets
 */
void run_user_input(struct list_head *list_commands, struct list_head *list_env, struct list_head *list_args, commandline cmdline, char *input, int argc) {

  check_PS1(list_env); 
  while(fgets(input, INPUT_LENGTH, stdin)) {
    if(input[0] != '\n' && input[0]!=' '){
      run_parser_executor_handler(list_commands, list_env, list_args, cmdline, input);
    }
    check_PS1(list_env);
  }

}