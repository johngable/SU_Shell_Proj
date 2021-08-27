#ifndef RUNNER_H
#define RUNNER_H

#include "list.h"
#include "datastructures.h"
#include "executor.h"
#include "internal.h" 
#include "environ.h"

#define INPUT_LENGTH 4094 // Max input length for strings

void run_rc_file(struct list_head *list_commands, struct list_head *list_env, struct list_head *list_args, commandline cmdline, char *input);
void run_user_input(struct list_head *list_commands, struct list_head *list_env, struct list_head *list_args, commandline cmdline, char *input, int argc); 

#endif 