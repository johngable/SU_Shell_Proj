#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "datastructures.h"
#include "list.h"

void run_command(int subcommand_count, struct list_head *list_commands, char **env);
#endif