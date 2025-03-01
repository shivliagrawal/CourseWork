#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "shell.h"

extern char ** environ;

/*
 *  Print all environment variables
 */

void printenv_cmd() {
  char ** env = environ;
  while (*env) {
    printf("%s\n", *env);
    env++;
  }
} /* printenv_cmd() */

/*
 *  Initialize a command_t
 */

void create_command(command_t *command) {
  command->single_commands = NULL;

  command->out_file = NULL;
  command->in_file = NULL;
  command->err_file = NULL;

  command->append_out = false;
  command->append_err = false;
  command->background = false;

  command->num_single_commands = 0;
} /* create_command() */

/*
 *  Insert a single command into the list of single commands in a command_t
 */

void insert_single_command(command_t *command, single_command_t *simp) {
  if (simp == NULL) {
    return;
  }

  command->num_single_commands++;
  int new_size = command->num_single_commands * sizeof(single_command_t *);
  command->single_commands = (single_command_t **)
                              realloc(command->single_commands,
                                      new_size);
  command->single_commands[command->num_single_commands - 1] = simp;
} /* insert_single_command() */

/*
 *  Free a command and its contents
 */

void free_command(command_t *command) {
  for (int i = 0; i < command->num_single_commands; i++) {
    free_single_command(command->single_commands[i]);
  }

  if (command->out_file) {
    free(command->out_file);
    command->out_file = NULL;
  }

  if (command->in_file) {
    free(command->in_file);
    command->in_file = NULL;
  }

  if (command->err_file) {
    free(command->err_file);
    command->err_file = NULL;
  }

  command->append_out = false;
  command->append_err = false;
  command->background = false;

  free(command);
} /* free_command() */

/*
 *  Print the contents of the command in a pretty way
 */

void print_command(command_t *command) {
  printf("\n\n");
  printf("              COMMAND TABLE                \n");
  printf("\n");
  printf("  #   single Commands\n");
  printf("  --- ----------------------------------------------------------\n");

  // iterate over the single commands and print them nicely
  for (int i = 0; i < command->num_single_commands; i++) {
    printf("  %-3d ", i );
    print_single_command(command->single_commands[i]);
  }

  printf("\n\n");
  printf("  Output       Input        Error        Background\n");
  printf("  ------------ ------------ ------------ ------------\n");
  printf("  %-12s %-12s %-12s %-12s\n",
         command->out_file ? command->out_file : "default",
         command->in_file ? command->in_file : "default",
         command->err_file ? command->err_file : "default",
         command->background ? "YES" : "NO");
  printf("\n\n");
} /* print_command() */

/*
 *  Handle built-in commands such as cd, setenv, and unsetenv
 */

int builtin_cmd(command_t * command, int i) {
  if (strncmp(command->single_commands[i]->arguments[0], "setenv", 6) == 0) {
    int err = setenv(command->single_commands[i]->arguments[1],
                     command->single_commands[i]->arguments[2], 1);

    if (err) {
      perror("setenv");
    }

    free_command(command);
    print_prompt();
    return 1;
  }

  if (strncmp(command->single_commands[i]->arguments[0], "unsetenv", 8) == 0) {
    int err = unsetenv(command->single_commands[i]->arguments[1]);

    if (err) {
      perror("unsetenv");
    }

    free_command(command);
    print_prompt();
    return 1;
  }

  if (strncmp(command->single_commands[i]->arguments[0], "cd", 2) == 0) {
    int dir;

    if (command->single_commands[i]->num_args == 1) {
      dir = chdir(getenv("HOME"));
    } else {
      dir = chdir(command->single_commands[i]->arguments[1]);
    }

    if (dir < 0) {
      fprintf(stderr, "cd: can't cd to %s",
              command->single_commands[i]->arguments[1]);
    }

    free_command(command);
    print_prompt();
    return 1;
  }

  return 0;
} /* builtin_cmd() */

/*
 *  Execute a command
 */

void execute_command(command_t *command) {
  // Don't do anything if there are no single commands
  if (command->single_commands == NULL) {
    if (isatty(0)) {
      print_prompt();
    }
    return;
  }

  int ret;

  int tmpin = dup(0);
  int tmpout = dup(1);
  int tmperr = dup(2);

  int fdin = 0;
  int fdout = 0;
  int fderr = 0;

  if (command->in_file) {
    fdin = open(command->in_file, O_RDONLY, 0600);
  } else {
    fdin = dup(tmpin);
  }

  for (int i = 0; i < command->num_single_commands; i++) {
    command->single_commands[i]->arguments[command->single_commands[i]->num_args] = NULL;

    if (builtin_cmd(command, i)) {
      return;
    }

    if (strncmp(command->single_commands[i]->executable, "exit", 4) == 0) {
      exit(0);
    }

    dup2(fdin, 0);
    close(fdin);

    if (i == ((command->num_single_commands) - 1)) {
      if (command->out_file) {
        if (command->append_out) {
          fdout = open(command->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
          fdout = open(command->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
      } else {
        fdout = dup(tmpout);
      }

      if (command->err_file) {
        if (command->append_err) {
          fderr = open(command->err_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
          fderr = open(command->err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
      } else {
        fderr = dup(tmperr);
      }
    } else {
      int fdpipe[2];
      pipe(fdpipe);
      fdout = fdpipe[1];
      fdin = fdpipe[0];
    }

    dup2(fdout, 1);
    dup2(fderr, 2);
    close(fdout);
    close(fderr);

    ret = fork();

    if (ret == 0) {
      if (strcmp(command->single_commands[i]->arguments[0], "printenv") == 0) {
        printenv_cmd();
        exit(0);
      }
      execvp(command->single_commands[i]->arguments[0], command->single_commands[i]->arguments);
      perror("execvp");
      exit(1);
    } else if (ret < 0) {
      perror("fork");
      exit(2);
    }
  }

  dup2(tmpin, 0);
  dup2(tmpout, 1);
  dup2(tmperr, 2);
  close(tmpin);
  close(tmpout);
  close(tmperr);

  if (!command->background) {
    waitpid(ret, NULL, 0);
  }

  // Clear to prepare for next command
  free_command(command);

  // Print new prompt
  if (isatty(0)) {
    print_prompt();
  }
} /* execute_command() */

