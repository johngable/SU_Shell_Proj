/**
 * @file executor.c
 * @author Hannah Moats 
 * @author John Gable 
 * @author Isabella Boone
 * @brief Handles the execution of the commands, inlcuding pipes
 * @version 0.1
 * @date 2021-03-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#define _GNU_SOURCE
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h> // for memory allocation
#include <stdio.h> // for input/output

#include "executor.h"
#include "error.h"

//Defined function for error printing
#define ERROR_CHECK_MSG(error, subcmd_file) ({ \
if (access(subcmd_file, F_OK) != 0) {          \
  fprintf(stderr, error, strerror(errno));     \
  return -1;                                   \
  }                                            \
})                                             \

/**
 * @brief Handles the execution of the child process 
 * @author Hannah Moats 
 * @param command char* Type of command being executed 
 * @param args char* const The list of args sent to exec 
 * @param env char** array of environment variables
 */
static void handleChildInExecutor(char *command, char *const *args, char **env) {
  pid_t pid2 = execvpe(command, args, env); // Execute command 
  fprintf(stderr, ERROR_EXEC_FAILED, strerror(errno)); // Should only be reaches when error happens with execvpe
  exit(1); // Exit with error 
}

/**
 * @brief Handles the execution of the parent process 
 * @author Hannah Moats
 * @param pid The process id
 * @param option The option passed to waitpid 
 */
static void handleParentInExecutor(pid_t pid, int option) {
  int status; 
  waitpid(pid, &status, option); // Wait for child to die
  // printf("Child exited: %d\n", status);
}

/**
 * @brief Handles getting input from a file, outputting to a file for commands. 
 *
 * @param subcmd The command who's input and output is being handled. 
 * @return int Returns -1 for error, 0 no error
 */
static int handle_input_output(struct subcommand *subcmd) {
  // If subcmd->output is anything other than stdout (default)
  if (strcmp(subcmd->output, "stdout") != 0) {
    const char *filename = subcmd->output; // Get filename
    int fd = 0; // File descriptor 

    // If type is truncate, open file with trucate flags
    if (subcmd->type == REDIRECT_OUTPUT_TRUNCATE) {
      fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777); 
       ERROR_CHECK_MSG(ERROR_EXEC_OUTFILE, subcmd->output);
    // If type is append, open file with append flags
    } else if (subcmd->type == REDIRECT_OUTPUT_APPEND) {
      fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0777); 
      ERROR_CHECK_MSG(ERROR_EXEC_APPEND, subcmd->output);
    }
    close(STDOUT_FILENO); // Close STDOUT
    dup2(fd, STDOUT_FILENO); // Connect FD and STDOUT 

  // If subcmd->input is anything other than "stdin" (default) 
  } if (strcmp(subcmd->input, "stdin") != 0) {
    const char *filename = subcmd->input; // Get filename
    FILE *file;
    file = fopen(filename, "r"); // File descriptor, open file read only
    int fd = fileno(file);  

    close(STDIN_FILENO); // Close STDIN
    dup2(fd, STDIN_FILENO); // Connect FD and STDIN
    close(fd);
  }
  return 0; 
}

/**
 * @brief Checks if the input file is in created. 
 * 
 * @param subcmd The subcommand that is being evaluated. 
 * @return int Returns -1 if the file is not vlaid, else returns 0. 
 */
static int check_validity_of_files(struct subcommand *subcmd) { 
  char *input = subcmd->input; 
  if (strcmp(input, "stdin") != 0) {
    ERROR_CHECK_MSG(ERROR_EXEC_INFILE, subcmd->input);
  }
  return 0; 
}

/**
 * @brief Executes the command, given the type of command and the argument array 
 * 
 * @param command The type of command that is being executed, Ex. /bin/ls
 * @param args The array of args that exec() takes in
 */
static void execute(char *command, char *const *args, struct subcommand *subcmd, char **env) { 
  pid_t pid = fork(); 

  if (pid == 0) { // Child process 
      // if there is output
      handle_input_output(subcmd);
      handleChildInExecutor(command, subcmd->exec_args, env);
  } else { // Parent process 
    handleParentInExecutor(pid, 0); 
  }
}


/**
 * @brief Runs the command typed on the command line including pipes
 * 
 * @param len The length of the linked listssss
 * @param list_args The linked list of args that are being executed 
 */
void run_command(int subcommand_count, struct list_head *list_commands, char **env) {
  struct subcommand *entry; 
  struct list_head *curr;  

  // If we have more than one subcommand, then we must use pipes
  if(subcommand_count>=2){
    int i=0; // Keep track of what subcommand we are on
    int prev_output=0; // Temp fd for saving previous pipe output
    int pipes[2]; // Standard pipes

    // Iterate through entire list until we reach the beggining again. 
    for (curr = list_commands->next; curr != list_commands; curr = curr->next) {
      entry = list_entry(curr, struct subcommand, list); // Update entry to look at current subcommand
      if (check_validity_of_files(entry) == -1) {
        return; 
      }
      //make pipes
      if(i<subcommand_count-1){
        int pipe_code = pipe(pipes);

        // If there was an error creating pipes
        if(pipe_code < 0){
          perror("Could not create pipes.\n");
          return;
        }
      }
      
      pid_t pid = fork(); 

      if (pid == 0) { // Child process 
        if(i<subcommand_count-1){ // All children except the last must write to parent
          dup2(prev_output, STDIN_FILENO); // read prev_output set by parent (initially empty)
          dup2(pipes[1], STDOUT_FILENO); // write output back to parent
        }else{ // The last child doesnt output to the parent for the loop 
          dup2(prev_output, STDIN_FILENO); // Read final pipe from parent
        }

        if ( handle_input_output(entry) != -1) {
          handleChildInExecutor(entry->exec_args[0], entry->exec_args, env); 
        }
        exit(1); 
      } else { // Parent process
        prev_output=pipes[0]; //get output from the child, and ensure that we can save it for next child
        close(pipes[1]); 
        handleParentInExecutor(pid, 0);
      }

      i++; //increment i so that we know which command we are on
    }
  } else{  // Else, if we only have one command 
    entry = list_entry(list_commands->next, struct subcommand, list); // Update entry to look at current subcommand
    if (check_validity_of_files(entry) == -1) {
      return; 
    }
    execute(entry->exec_args[0], entry->exec_args, entry, env); // Execute command
  }
}
