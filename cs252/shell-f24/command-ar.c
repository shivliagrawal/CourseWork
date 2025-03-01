/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 * DO NOT PUT THIS PROJECT IN A PUBLIC REPOSITORY LIKE GIT. IF YOU WANT
 * TO MAKE IT PUBLICALLY AVAILABLE YOU NEED TO REMOVE ANY SKELETON CODE
 * AND REWRITE YOUR PROJECT SO IT IMPLEMENTS FUNCTIONALITY DIFFERENT THAN
 * WHAT IS SPECIFIED IN THE HANDOUT. WE OFTEN REUSE PART OF THE PROJECTS FROM
 * SEMESTER TO SEMESTER AND PUTTING YOUR CODE IN A PUBLIC REPOSITORY
 * MAY FACILITATE ACADEMIC DISHONESTY.
 */


#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#include "shell.h"
#include "single_command.h"

#define TRUE   (1)
#define FALSE  (0)

/*
 * Function: create_command
 * ------------------------
 * Initializes a command_t structure by setting its fields to default values.
 *
 * command: A pointer to the command_t structure to initialize.
 */
void create_command(command_t *command) {
  command->single_commands = NULL;
  command->num_single_commands = 0;

  command->out_file = NULL;
  command->in_file = NULL;
  command->err_file = NULL;

  command->append_out = false;
  command->append_err = false;
  command->background = false;
} /* create_command() */

/*
 * Function: insert_single_command
 * -------------------------------
 * Inserts a single_command_t into the command_t's list of single commands.
 *
 * command: A pointer to the command_t structure.
 * simp:    A pointer to the single_command_t to insert.
 */
void insert_single_command(command_t *command, single_command_t *simp) {
  single_command_t **new_single_commands = NULL;
  int new_size = 0;

  if (simp == NULL) {
    return;
  }

  command->num_single_commands++;
  new_size = command->num_single_commands * sizeof(single_command_t *);

  new_single_commands = (single_command_t **)
    realloc(command->single_commands, new_size);

  if (new_single_commands == NULL) {
    fprintf(stderr, "Error: realloc failed in insert_single_command.\n");
    exit(EXIT_FAILURE);
  }

  command->single_commands = new_single_commands;
  command->single_commands[command->num_single_commands - 1] = simp;
} /* insert_single_command() */

/*
 * Function: free_command
 * ----------------------
 * Frees the memory allocated for a command_t structure and its contents.
 *
 * command: A pointer to the command_t structure to free.
 */
void free_command(command_t *command) {
  int i = 0;

  for (i = 0; i < command->num_single_commands; ++i) {
    free_single_command(command->single_commands[i]);
  }

  if (command->single_commands != NULL) {
    free(command->single_commands);
    command->single_commands = NULL;
  }

  if (command->out_file != NULL) {
    free(command->out_file);
    command->out_file = NULL;
  }

  if (command->in_file != NULL) {
    free(command->in_file);
    command->in_file = NULL;
  }

  if ((command->err_file != NULL) && (command->err_file != command->out_file)) {
    free(command->err_file);
    command->err_file = NULL;
  }

  command->append_out = false;
  command->append_err = false;
  command->background = false;

   free(command);
} /* free_command() */

/*
 * Function: execute_command
 * -------------------------
 * Executes the command_t by handling input/output redirection, pipes,
 * background execution, and forking processes for each single command.
 *
 * command: A pointer to the command_t structure to execute.
 */

void execute_command(command_t *command) {
  int tmp_in = 0;
  int tmp_out = 0;
  int tmp_err = 0;
  int fd_in = 0;
  int fd_out = 0;
  int fd_err = 0;
  int ret = 0;
  int i = 0;
  //int status = 0;

  /* Don't do anything if there are no single commands */
  if (command->num_single_commands == 0) {
    print_prompt();
    return;
  }

  /* Save default file descriptors */
  tmp_in = dup(0);
  tmp_out = dup(1);
  tmp_err = dup(2);

  if (command->in_file != NULL) {
    fd_in = open(command->in_file, O_RDONLY);
    if (fd_in < 0) {
      perror("stdin error");
      exit(1);
    }
  } else {
    fd_in = dup(tmp_in);
  }

  int fd_pipe[2];
  /* Iterate over each single command */
  for (i = 0; i < command->num_single_commands; ++i) {
    /* Redirect input for the current command */
    dup2(fd_in, 0);
    close(fd_in);

    /* Set up output and error redirection */
    if (i == command->num_single_commands - 1) {
      /* Last command */
      if (command->out_file != NULL) {
        if (command->append_out == TRUE) {
          fd_out = open(command->out_file, O_WRONLY | O_CREAT | O_APPEND, 0664);
        } else {
          fd_out = open(command->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        }
      } else {
        fd_out = dup(tmp_out);
      }

      if (command->err_file != NULL) {
        if (command->append_err == TRUE) {
          fd_err = open(command->err_file, O_WRONLY | O_CREAT | O_APPEND, 0664);
        } else {
          fd_err = open(command->err_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
        }
      } else {
        fd_err = dup(tmp_err);
      }
    } else {
      /* Not last command */
      //fd_err = dup(tmp_err);

      /* Create pipe */
      pipe(fd_pipe);
      fd_out = fd_pipe[1];
      fd_in = fd_pipe[0];
    }

    /* Redirect output */
    dup2(fd_out, 1);
    close(fd_out);
    dup2(fd_err, 2);
    //close(fd_err);

    /* Fork the process */
    ret = fork();
    if (ret == 0) {
      /* Child process */

      single_command_t *cmd = NULL;
      cmd = command->single_commands[i];

      close(tmp_in);
      close(tmp_out);
      close(tmp_err);
      execvp(cmd->arguments[0], cmd->arguments);

      /* If execvp returns, there was an error */
      perror(cmd->arguments[0]);
      exit(1);
    } else if (ret < 0) {
      /* Fork failed */
      perror("fork");
      exit(1);
      /* Restore default file descriptors */
      //dup2(tmp_in, 0);
      //dup2(tmp_out, 1);
      //dup2(tmp_err, 2);
      //close(tmp_in);
      //close(tmp_out);
      //close(tmp_err);
      //return;
    }
    close(fd_out);
  
  }
/* Restore default file descriptors */
      dup2(tmp_in, 0);
      dup2(tmp_out, 1);
      dup2(tmp_err, 2);
      close(tmp_in);
      close(tmp_out);
      close(tmp_err);

  if (command->background == FALSE) {
	  waitpid(ret, NULL, 0);
  }

  free_command(command);
  /* Print new prompt */
  print_prompt();
} /* execute_command() */
