
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "environ.h"
#include "datastructures.h"

/**
 * @brief Test that we can display a 2D array of environment variables
 * 
 * @param envp 
 */
void test_display_env_array(char ** envp) {
    display_env_array(envp); 
}

/**
 * @brief Test that we can make a list of environment variables
 * 
 * @param list 
 * @param envp 
 */
void test_make_env_list(struct list_head *list, char **envp) {
    make_env_list(list, envp); 
}

/**
 * @brief Test that we can display the entire list of environment variables
 * 
 * @param envp_list 
 */
void test_display_env_list(struct list_head *envp_list) {
    display_env_list(envp_list); 
}

/**
 * @brief Test that we can input a list of environment variables and
 * make the list into an array of variables
 * 
 * @param list 
 * @return char** 
 */
char ** test_make_env_array(struct list_head *list) {
    char **envp = make_env_array(list); 
    display_env_array(envp); 
    return envp; 
}

/**
 * @brief Test that we can clear our array of environment variables
 * 
 * @param envp 
 * @param list 
 */
void test_free_env_array(char **envp, struct list_head *list) {
    int len = getListLength(list); ;
    free_env_array(envp, len); 
}

/**
 * @brief Test that we can clear our list of environment variables out
 * 
 * @param list 
 */
void test_free_env_list(struct list_head *list) {
    clear_list_env(list); 
} 

/**
 * @brief Test to ensure we can get an internal environment variable from our list
 * 
 * @param list 
 */
void test_get_env(struct list_head *list) {
    char *env; 
    char *og; 

    og = getenv("NAME"); 
    printf("getenv NAME: %s\n", og); 

    env = get_env(list, "NAME"); 
    printf("get_env NAME: %s\n", env);  
}

/**
 * @brief Test to ensure we can add a new environment variable and value to the internal environment list
 * 
 * @param list 
 */
void test_set_env(struct list_head *list) {
    char *env; 
    set_env(list, "NAME", "batman"); 
    env = getenv("NAME"); 
    
}

/**
 * @brief Temp struct that reflects our subcommand struct
 * 
 */
struct subcommand_new {
    char **execs; 
    char *input; 
    char *output; 
    enum Token type; 
    struct list_head list; 
}; 

/**
 * @brief Test to see if we can successfully print the list of subcommands
 * 
 * @param list_commands 
 */
void test_print_list(struct list_head *list_commands) {
   struct list_head *curr; 
   curr = list_commands->next; 
   struct subcommand_new *sub = list_entry(curr, struct subcommand_new, list); 
   printf("%s\n", sub->execs[0]); 
   printf("%s\n", sub->execs[1]); 

    pid_t pid = fork(); 

    if (pid == 0) {
        //concat /bin/ls 
        execvp("/bin/ls", (char *const *)sub->execs); 
        exit(1);  
    } else {
        int status; 
        waitpid(pid, &status, 0); 
    }

}

/*
//Commented out to compile sush without main yelling
int main (int argc, char **argv, char **envp) {

    
    LIST_HEAD(list_envp); 

    
    printf("Test 1: \n"); 
    test_display_env_array(envp);

    printf("\nTest 2: \n");
    test_make_env_list(&list_envp, envp); 
    //test_display_env_list(&list_envp); 
    //free_env_list(&list_envp); 
    
    printf("\nTest 3: \n");
    char **envp_test = test_make_env_array(&list_envp);

    printf("\nTest 4: \n"); 
    test_get_env(&list_envp);

    printf("\nTest 5: \n");  
    test_set_env(&list_envp);

    printf("\nTest next to last: \n"); 
    test_free_env_array(envp_test, &list_envp); 
    printf("Freed??? %s\n", envp_test[0]); //yay it works
    

    printf("\nTest last: \n"); 
    test_free_env_list(&list_envp); 
    display_env_list(&list_envp); //yay it works 

    
   LIST_HEAD(list_args); 
   LIST_HEAD(list_commands); 

   char *args[] = {"ls", "-l", ">", "output.txt", NULL}; 


   struct subcommand_new *subcmd = malloc(sizeof(struct subcommand_new)); 
   //scan list_arg for redirect, mark the input or output as either a file or stdin or stdout
   //if there is more than one command then you know its pipes
   //for the first command, its either stdin or stdout or file input
   //for the last command its either stdin or stdout or file output 
   //for the middle commands its always stdin or stdout
   //assign the toke type to the token type of the subcommand
    subcmd->input = strdup("stdin"); 
    subcmd->output = strdup(args[3]); 
    subcmd->type = REDIRECT_OUTPUT_TRUNCATE; 
    //remove, file names and redirects from list


   int num_args = 2 + 1; //for the NULL
   subcmd->execs = malloc(num_args * sizeof(char *)); 
   subcmd->execs[0] = args[0]; 
   subcmd->execs[1] = args[1]; 
   subcmd->execs[2] = NULL; 
   printf("%s\n", subcmd->execs[0]); 
   printf("%s\n", subcmd->execs[1]); 
   //printf("%s\n", subcmd->execs[2]); 
   //delte all nodes up until the NULL
   //repeat 

   list_add_tail(&subcmd->list, &list_commands); 

   test_print_list(&list_commands); 
   
    
    test_make_env_list(&list_envp, envp); 

    set_env(&list_envp, "PS1", ">"); 
    display_env_list(&list_envp); 
    
    return 0; 
} */

