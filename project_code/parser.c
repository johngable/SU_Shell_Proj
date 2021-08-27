/**
 * @file parser.c
 * @author Hannah Moats 
 * @author John Gable 
 * @author Isabella Boone
 * @brief Parses the command line and fills in the list_commands which is a list 
 * of subcommands. Each subcommand, contains info on input and output, and a 2D array. 
 * @version 0.1
 * @date 2021-03-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "list.h"
#include "datastructures.h"
#include "internal.h"
#include "error.h"

#define MAX_BUFFER 4096

/**
 * @brief Special characters that our parser must account fo with specific actions
 * 
 */
#define SPACE ' '
#define TAB '\t'
#define NEWLINE '\n'
#define QUOTATIONMARK '"'
#define PIPE '|'
#define REDIR_IN '<'
#define REDIR_OUT '>'

#define DELETE_FILE(entry, path, curr) ({ \
  entry = list_entry(curr, argument, list); \
  curr = curr->next; \
  free(subcommand->path); \
  subcommand->path = strdup(entry->contents); \
  free(entry->contents); \
  list_del(&entry->list); \
  free(entry); \
}) 

/**
 * @brief Current state off parser 
 */
enum State
{
  START,
  WHITESPACE,
  CHARACTER,
  QUOTE,
  OUTPUT,
  REDIR,
  INPUT
};

/**
 * @brief Find the number of subcommands in the input string and returns that value. 
 * @author Hannah Moats 
 * 
 * @param input String to search through
 * @param len int length of String
 * @return int number of subcommands found
 */
int find_num_subcommands(char input[], int len)
{
  int count = 1; // Number of subcommands counted 
  for (int i = 0; i < len; i++) // For every char in the string
  {
    if (input[i] == PIPE)
    {
      count++;
    }
  }
  return count;
}

/**
 * @brief Copy an individual subcommand to a pointer
 * @author Hannah Moats  
 * 
 * @param subcommand 2D char array to copy from.
 * @param sentence char * destination to copy to
 * @param i which sentence in subcommand to copy
 */
static void copy_subcommand(char **subcommand, char *destination, int i)
{
  strcpy(subcommand[i], destination); // Copy subcommand to destination
}

/**
 * @brief Copy a String of subcommands into a 2D array of subcommands. 
 * @author Hannah Moats 
 * 
 * @param input String to break apart
 * @param num int number of subcommands in String
 * @param subcommand 2D char array to copy subcommands into
 */
void copy_subcommands(char input[], int num, char **subcommand)
{
  // If is a newline
  if(input[0]!=NEWLINE){ 
    int i, len;
    char *cmd = strtok(input, "|"); // Take the command before the pipe 
    
    if (cmd != NULL) {
      len = strlen(cmd); // Get length of command 
    } else {
      return; 
    }

    for (i = 0; i < num; i++)
    { 
      if (cmd != NULL) {
        len = strlen(cmd); // Get length of command 
      } else {
        return; 
      }

      subcommand[i] = malloc(len + 2); // ALlcoate space 
      copy_subcommand(subcommand, cmd, i); // Copy subcommand

      cmd = strtok(NULL, "|"); 
    }
  }
}

/**
 * @brief Traverse through linked list and free it from memory
 * after deleting each node entry. 
 * 
 * @param list struct list_head to clear
 */
static void clear_list_argument(struct list_head *list)
{
  argument *entry; //Current entry during traversal

  // While list is not empty
  while (!list_empty(list))
  {
    // Delete entry from list and free its contents and itself.
    entry = list_entry(list->next, argument, list);
    list_del(&entry->list);
    free(entry->contents);
    free(entry);
  }
}

/**
 * @brief Check whether or not a char is considered whitespace.
 * 
 * @param c char to check
 * @return int 1 if it is considered whitespace, 0 if it is not. 
 */
static int is_whitespace(char c){
  if(c == SPACE || c == TAB){
    return 1;
  }
  return 0;
}

/**
 * @brief Check whether or not a char is considered a quote. 
 * 
 * @param c char to check. 
 * @return int 1 if it is considered a quote, 0 if it is not. 
 */
static int is_quote(char c){
  if(c == QUOTATIONMARK){
    return 1;
  }
  return 0;
}

/**
 * @brief Check whether or not a char is considered a redirect symbol. 
 * 
 * @param c char to check.
 * @return int 1 if it is considered a redirect symbol, 0 if it is not. 
 */
static int is_redir(char c){
  if(c == REDIR_IN || c == REDIR_OUT ){
    return 1;
  }
  return 0;
}

/**
 * @brief Checks whether or not a char is considered a character. 
 * 
 * @param c char to check
 * @return int 1 if it is considered character, 0 if it is not. 
 */
static int is_character(char c){
  if(!is_redir(c) && !is_whitespace(c) && !is_quote(c)){
    return 1;
  }

  return 0;
}

/**
 * @brief Check character state of a char.  
 * 
 * @param c char to check. 
 * @return int return 1: whitespace, 2: character, 3: quote, 5:redir
 */
static int check_character_state(char c){
  if(is_character(c)){
    return CHARACTER;
  }else if(is_whitespace(c)){
    return WHITESPACE;
  }else if(is_quote(c)){
    return QUOTE;
  }else if(is_redir(c)){
    return REDIR;
  }
}

/**
 * @brief Deletes a token from the list and marks the subcommand with token type. 
 * 
 * @param entry The current entry from the argument list. 
 * @param subcommand The subcommand that is being altered.  
 */
static void delete_token(argument *entry, struct subcommand *subcommand) {
  subcommand->type = entry->token; 
  free(entry->contents); 
	list_del(&entry->list); 
	free(entry); 
}

/**
 * @brief Takes in the arg list, sets the input, output, and type for the subcommand. 
 * Also removes any filesnames or redirects from the list_args so it can be processed 
 * and made into an array for exec. 
 * 
 * @param arg The list of args from the command line. Will look like "ls", "-l", "\0"
 * @param subcommand The sub command of the commandline that is being "filled" in 
 */
static void get_input_output(struct list_head *arg, struct subcommand *subcommand) {  
  // Assign default values to the struct 
  subcommand->input = strdup("stdin");    
  subcommand->output = strdup("stdout"); 
  subcommand->type = NORMAL; 
  
  struct list_head *curr = arg->next; 
  argument *entry; 
  int i = 0; 

  while (curr != arg && i < 20) {
    i++; 
    entry = list_entry(curr, argument, list); 
    curr = curr->next; 
    if (entry->token == REDIRECT_OUTPUT_TRUNCATE || entry->token == REDIRECT_OUTPUT_APPEND) {
      delete_token(entry, subcommand); 
      DELETE_FILE(entry, output, curr); 
    } else if (entry->token == REDIRECT_INPUT) {
      delete_token(entry, subcommand);
      DELETE_FILE(entry, input, curr); 
    } 
  }
}

/**
 * @brief Takes the remaining arguments of list_args and stores them as a 2D array
 * in subcommand. 
 * 
 * @param list_args The list of args that is being placed in an array 
 * @param sub The subcommand of the commandline that the array belongs to
 */
static void make_exec_args_array(struct list_head *list_args, struct subcommand *sub) {
  int num_args = getListLength(list_args); 
  sub->exec_args = malloc(num_args * sizeof(char *)); 
  // Loop through args list and assign exec_args[i] the value of contents
  struct list_head *curr;  
  argument *entry; 
  int i = 0; 
    
  for (curr = list_args->next; curr != list_args->prev; curr = curr->next) {
      entry = list_entry(curr, argument, list); 
      sub->exec_args[i] = strndup(entry->contents, strlen(entry->contents)); 
      i++; 
  }
  sub->exec_args[num_args-1] = NULL; 
}

/**
 * @brief Checks to see if the command from input is an internal command
 * 
 * @param arg1 The name of the command
 * @return int Returns 0 if the command is an internal command, else 1 if it is not
 */
static int check_internal_command(struct list_head *list_args) {
  char *internal_cmds[] = {"setenv", "getenv", "unsetenv", "cd", "pwd", "exit"};
  argument *entry = list_entry(list_args->next, argument, list); 
  int i; 
  for (i = 0; i < 6; i++) {
    if (strcmp(internal_cmds[i], entry->contents) == 0) {
      return 0; 
    }
  }
  return 1; 
}

/**
 * @brief Constructs a struct that holds information about the individual subcommand of 
 * from the command line. The subcommand is added to a list of commands, which represents the entire 
 * command line. 
 * 
 * @param list_commands The list which the subcommand will be added to
 * @param list_args The parsed argument list that is being turned into a subcommand. 
 */
static void make_subcommand(struct list_head *list_commands, struct list_head *list_args) {
  struct subcommand *sub = malloc(sizeof(struct subcommand)); 

  if(check_internal_command(list_args)){
      get_input_output(list_args, sub); //fills in the struct fields: input, output, type
  } else {
    sub->input = strdup("stdin"); 
    sub->output = strdup("stdout"); 
  }
  make_exec_args_array(list_args, sub); //fills in the struct field: exec_args ==> "ls", "-l", NULL
  list_add_tail(&sub->list, list_commands); 
}

/**
 * @brief Adds a parsed arg to the list of args. The list of args is used to make the 
 * subcommand struct. Also used to mark the token value. 
 * 
 * @param temp The item being added
 * @param token The type of arg it is
 * @param arg The struct which stores the argument
 * @param list_args The list which contains the args
 */
static void add_arg_to_list(char *temp, int token, argument *arg, struct list_head *list_args){
  arg = malloc(sizeof(argument)); 
  arg->contents = strdup(temp); // Copy temp to contents
  arg->token = token; // Set token to normal
  list_add_tail(&arg->list, list_args); // Add to the end of the list
  memset(temp, 0, 50);
}

/**
 * @brief Ensures that the commandline appropiately uses the redirect operators. 
 * @author Hannah Moats 
 * 
 * @param args The current commandline argument being evaluated 
 * @param total_cmds The total number of subcommands on the commandline
 * @return int Returns -1, if command line error, else returns 0 with no error 
 */
static int check_validity_of_cmdline_redirects(struct list_head *list_args, int total_cmds, int current_cmd, int stdins, int stdouts) {
  if (total_cmds == 1) {
    //can't have two standard outs 
    //can't have more than one standard ins
    if (stdins > 1 || stdouts > 1) {
      fprintf(stderr, ERROR_INVALID_CMDLINE); 
      return -1; 
    }
  } else if (total_cmds > 1 && current_cmd == 1) { 
    if (stdins > 1 || stdouts != 0) {
      fprintf(stderr, ERROR_INVALID_CMDLINE);
      return -1;
    }
  } else if (total_cmds > 1 && current_cmd < total_cmds) {
    if (stdins != 0 || stdouts != 0) {
      fprintf(stderr, ERROR_INVALID_CMDLINE);
      return -1; 
    }
  } else if (total_cmds > 1 && current_cmd == total_cmds) {
    if (stdins != 0 || stdouts > 1) {
      fprintf(stderr, ERROR_INVALID_CMDLINE);
      return -1; 
    }
  }
  return 0; 
}

/**
 * @brief Frees the argument list used to store the parsed argument values. 
 * Frees the temp value used in scanning the arguments. 
 * 
 * @param list_args The list of parsed arguments. 
 * @param temp The temporary buffer. 
 */
static void free_malloced_parser_values(struct list_head *list_args, char *temp) {
  clear_list_argument(list_args); 
  free(temp);
  return; 
}

/**
 * @brief Parses the character array from user input (stored in commandlline) and
 * creates a list of commands, or subcommand structs, where each 
 * subcommand is parsed and marked with the appropiate input and output. 
 * 
 * @param list_args A list used to store the parsed arguments of a subcommand
 * @param commandline The struct which stores unparsed subcommands, and number of subcommands. 
 * @param list_commands The list that stores the subcommands 
 * @return int Returns -1 if there was an error, else returns 0
 */
int parse_commandline(struct list_head *list_args, commandline *commandline, struct list_head *list_commands)
{
  int word_count = 0; //Count for how many words we have parsed out of the commandline sentences
  int currentState = WHITESPACE; // Start in whitespace state by default
  int redirect_in_count = 0; 
  int redirect_out_count = 0; 

  char *temp = calloc(100, sizeof(char)); // Temporary word variable
  argument *arg; // Linked List of arguments
  
  // For every subcommand 
  for (int i = 0; i < commandline->num; i++)
  {
    redirect_in_count = 0; 
    redirect_out_count = 0; 
    // For every character in the subcommand
    for (int j = 0; j < strlen(commandline->subcommand[i]); j++){
      char current_character = commandline->subcommand[i][j]; // Purely for readability's sake
      int current_characters_state = check_character_state(current_character);  //check for the current_characters type

      // If the current char is not a space, tab, or quotation mark
      if (current_characters_state == CHARACTER || current_characters_state == REDIR) {
        // If we encounter redirect symbol
        if (current_character == REDIR_OUT) {

          if(strlen(temp)>0){
            add_arg_to_list(temp, NORMAL, arg, list_args);
          }

          // If we encounter two arrow redir_out symbol ">>"
          if (commandline->subcommand[i][j + 1] == REDIR_OUT) {
            strncat(temp, &commandline->subcommand[i][j], 2); // Copy symbols to temp
            add_arg_to_list(temp, REDIRECT_OUTPUT_APPEND, arg, list_args);
            j++;
            redirect_out_count++; 
          } else {
            // Otherwise, we only encountered one redir_out '>'
            strncat(temp, &commandline->subcommand[i][j], 1); // Copy symbol to temp
            add_arg_to_list(temp, REDIRECT_OUTPUT_TRUNCATE, arg, list_args);
            redirect_out_count++; 
          }
        } else if (current_character == REDIR_IN) {
          // If we encounter the redirect output symbol

          //If the temp var already has an argument in it prior to redir_in
          if(strlen(temp)>0){
            add_arg_to_list(temp, NORMAL, arg, list_args);
          }
          
          strncat(temp, &commandline->subcommand[i][j], 1); // Copy symbol to temp

          //Always add the redir_in to the list
          add_arg_to_list(temp, REDIRECT_INPUT, arg, list_args);
          redirect_in_count++; 
          
        } else {
          // Current char is a character
          currentState = CHARACTER; // Set current state
          strncat(temp, &commandline->subcommand[i][j], 1); // Copy character

          // if we found the last word, and it has no space after, add it to the list
          if (j == (strlen(commandline->subcommand[i]) - 1)) {
            add_arg_to_list(temp, NORMAL, arg, list_args);
          }
        } 
      } else if (current_characters_state == WHITESPACE) {
        // If we see a space or tab
        if (currentState != WHITESPACE) {
          add_arg_to_list(temp, NORMAL, arg, list_args);  //add the current content of temp to the list of args
          currentState = WHITESPACE;
          word_count++;                    //increment which word we are on
        }
      } else if (current_characters_state == QUOTE) {
        if (currentState != QUOTE) {
          currentState = QUOTE;
          j++;
          while (commandline->subcommand[i][j] != QUOTATIONMARK && j < strlen(commandline->subcommand[i]))
          {
            strncat(temp, &commandline->subcommand[i][j], 1);
            j++;
          }
          if (j >= strlen(commandline->subcommand[i])) {
            free_malloced_parser_values(list_args, temp); 
            fprintf(stderr, ERROR_INVALID_CMDLINE); 
            return -1; 
          }

          add_arg_to_list(temp, NORMAL, arg, list_args);
          currentState = WHITESPACE;
        }
      }
    }

    //The req specify ending the list of arguements with a NULL for exec
    arg = malloc(sizeof(argument));
    arg->contents = strdup("\0");
    arg->token = NORMAL;
    list_add_tail(&arg->list, list_args);

    //Check for malformed commandline
    int error_check = check_validity_of_cmdline_redirects(list_args, commandline->num, i + 1, redirect_in_count, redirect_out_count); 
    if (error_check == -1) {
      clear_list_argument(list_args); 
      free(temp); 
      return -1; 
    }
    
    //Makes a subcomamnd, then clears the list_args so that more args can be scanned
    //at this point list_args == "ls" "-l" "\0" 
    make_subcommand(list_commands, list_args); 
    clear_list_argument(list_args); 
  }
  free(temp);
  return 0; 
}
