/**
 * @file datastructures.h
 * @author Isabella Boone 
 * @author John Gable
 * @author Hannah Moats
 * @brief This file contains all data structures and enums needed 
 *   for holding a command line argument. 
 * @version 0.1
 * @date 2021-03-25
 * 
 * @copyright Copyright (c) 2021
 * 
 */

// File Guard
#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

// Imports
#include <string.h> // for strings
#include "list.h" // for navigating lists

/**
 * @brief Enum to describe what type of argument is held. 
 */
enum Token
{
  REDIRECT_INPUT,
  REDIRECT_OUTPUT_APPEND,
  REDIRECT_OUTPUT_TRUNCATE,
  NORMAL,
  FILENAME
};

/**
 * @brief Enum to describe the status of the job commands.  
 */
enum Job_Status {
  COMPLETE, QUEUED, RUNNING
}; 

/**
 * @brief Struct to hold individual arguments in a subcommand. 
 * 
 * @param contents the input
 * @param Token enum to describe what type of argument we have
 * @param list_head list to navigate through, linking to next and prev argument
 */
typedef struct Argument {
    char *contents; // Contents
    enum Token token; 
    struct list_head list;
} argument; 


/**
 * @brief Commandline - the full line of input from the user
 * 
 * @param num int number of subcommands
 * @param subcommand 2D char array of all subcommands
 * @param stdin 2D char array to hold where input of the function comes from
 * @param stdout 2D char array to hold where the command outputs to
 */
typedef struct Commandline {
  int num; 
  char **subcommand; 
} commandline;

/**
 * @brief subcommand - a sub command from the full commandline (sub commands are split at the pipes)
 * 
 * @param exec_args A parsed subcommand ending with NULL
 * @param input The input of the command (stdin, file name)
 * @param output The output of the command (stdout, file name)
 * @param type The type of redirect
 * @param list The list which subcommand points to
 */
struct subcommand {
    char **exec_args; 
    char *input; 
    char *output; 
    enum Token type; 
    struct list_head list; 
}; 

/**
 * @brief The struct which holds all the information on the job command. 
 */
struct job_command {
  char **exec_args; ///< The 2D array sent to args
  char *output_file; ///< the output file where the command outputs 
  enum Job_Status status; ///< the current status of the job
  int position; ///< the position of the job in the queue
  int process_id; ///< the process ID that the job is running on
  struct list_head queue; ///< the queue that the job belongs to
};

/**
 * @brief Find the number of subcommands in the input string and returns that value. 
 * 
 * @param input String to search through
 * @param len int length of String
 * @return int number of subcommands found
 */
int find_num_subcommands(char input[], int len);

/**
 * @brief Copy a String of subcommands into a 2D array of subcommands. 
 * 
 * @param input String to break apart
 * @param num int number of subcommands in String
 * @param subcommand 2D char array to copy subcommands into
 */
void copy_subcommands(char input[], int num, char **subcommand);

/**
 * @brief Creates a list of commands, or subcommand structs, where each 
 * subcommand is parsed and marked with the appropiate input and output. 
 * 
 * @param list_args A list used to store the parsed arguments of a subcommand
 * @param commandline The struct which stores unparsed subcommands, and number of subcommands. 
 * @param list_commands The list that stores the subcommands 
 * @return int Returns -1 if there was an error, else returns 0
 */
int parse_commandline(struct list_head *list_args, commandline *commandline, struct list_head *list_commands);

#endif