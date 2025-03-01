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
  int status = 0;

  /* Don't do anything if there are no single commands */
  if (command->num_single_commands == 0) {
    print_prompt();
    return;
  }

  /* Save default file descriptors */
  tmp_in = dup(0);
  tmp_out = dup(1);
  tmp_err = dup(2);

  if (tmp_in < 0 || tmp_out < 0 || tmp_err < 0) {
    fprintf(stderr, "Error: dup failed in execute_command.\n");
    exit(EXIT_FAILURE);
  }

  if (command->in_file != NULL) {
    fd_in = open(command->in_file, O_RDONLY);
    if (fd_in < 0) {
      perror(command->in_file);

      /* Restore default file descriptors */
      dup2(tmp_in, 0);
      dup2(tmp_out, 1);
      dup2(tmp_err, 2);
      close(tmp_in);
      close(tmp_out);
      close(tmp_err);
      return;
    }
  } else {
    fd_in = dup(tmp_in);
    if (fd_in < 0) {
      fprintf(stderr, "Error: dup failed for fd_in in execute_command.\n");
      exit(EXIT_FAILURE);
    }
  }

  /* Iterate over each single command */
  for (i = 0; i < command->num_single_commands; ++i) {
    /* Redirect input for the current command */
    if (dup2(fd_in, 0) < 0) {
      fprintf(stderr, "Error: dup2 failed for fd_in in execute_command.\n");
      exit(EXIT_FAILURE);
    }
    close(fd_in);

    /* Set up output redirection */
    if (i == command->num_single_commands - 1) {
      /* Last command */
      if (command->out_file != NULL) {
        if (command->append_out == TRUE) {
          fd_out = open(command->out_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
        } else {
          fd_out = open(command->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
        if (fd_out < 0) {
          perror(command->out_file);

          /* Restore default file descriptors */
          dup2(tmp_in, 0);
          dup2(tmp_out, 1);
          dup2(tmp_err, 2);
          close(tmp_in);
          close(tmp_out);
          close(tmp_err);
          return;
        }
      } else {
        fd_out = dup(tmp_out);
        if (fd_out < 0) {
          fprintf(stderr, "Error: dup failed for fd_out in execute_command.\n");
   //       exit(EXIT_FAILURE);
        }
      }
    } else {
      /* Not last command, create pipe */
      int fd_pipe[2] = {0};

      if (pipe(fd_pipe) == -1) {
        perror("pipe");

        /* Restore default file descriptors */
        dup2(tmp_in, 0);
        dup2(tmp_out, 1);
        dup2(tmp_err, 2);
        close(tmp_in);
        close(tmp_out);
        close(tmp_err);
        return;
      }
      fd_out = fd_pipe[1];
      fd_in = fd_pipe[0];
    }

    /* Redirect output */
    if (dup2(fd_out, 1) < 0) {
      fprintf(stderr, "Error: dup2 failed for fd_out in execute_command.\n");
 //     exit(EXIT_FAILURE);
    }
    close(fd_out);

    /* Set up error redirection */
    if (command->err_file != NULL) {
      if (command->append_err == TRUE) {
        fd_err = open(command->err_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
      } else {
        fd_err = open(command->err_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      }
      if (fd_err < 0) {
        perror(command->err_file);

        /* Restore default file descriptors */
        dup2(tmp_in, 0);
        dup2(tmp_out, 1);
        dup2(tmp_err, 2);
        close(tmp_in);
        close(tmp_out);
        close(tmp_err);
        return;
      }
    } else {
      fd_err = dup(tmp_err);
      if (fd_err < 0) {
        fprintf(stderr, "Error: dup failed for fd_err in execute_command.\n");
        exit(EXIT_FAILURE);
      }
    }

    /* Redirect stderr */
    if (dup2(fd_err, 2) < 0) {
      fprintf(stderr, "Error: dup2 failed for fd_err in execute_command.\n");
      exit(EXIT_FAILURE);
    }
    close(fd_err);

    /* Fork the process */
    ret = fork();
    if (ret == 0) {
      /* Child process */
      single_command_t *cmd = NULL;
      cmd = command->single_commands[i];

      execvp(cmd->arguments[0], cmd->arguments);

      /* If execvp returns, there was an error */
      perror(cmd->arguments[0]);
      exit(EXIT_FAILURE);
    } else if (ret < 0) {
      /* Fork failed */
      perror("fork");

      /* Restore default file descriptors */
      dup2(tmp_in, 0);
      dup2(tmp_out, 1);
      dup2(tmp_err, 2);
      close(tmp_in);
      close(tmp_out);
      close(tmp_err);
      return;
    }

    /* Restore stderr in parent */
    if (dup2(tmp_err, 2) < 0) {
      fprintf(stderr, "Error: dup2 failed to restore stderr in execute_command.\n");
      exit(EXIT_FAILURE);
    }

    /* For the next command, input is from the pipe we set up */
    /* If there is a next command, fd_in is already set to fd_pipe[0] */
  }

  /* Restore default file descriptors */
  if (dup2(tmp_in, 0) < 0 ||
      dup2(tmp_out, 1) < 0 ||
      dup2(tmp_err, 2) < 0) {
    fprintf(stderr, "Error: dup2 failed to restore file descriptors in execute_command.\n");
    exit(EXIT_FAILURE);
  }
  close(tmp_in);
  close(tmp_out);
  close(tmp_err);

  if (command->background == FALSE) {
    /* Wait for all child processes */
    while (wait(&status) > 0) {
      /* Waiting for child processes to finish */
    }
  }

  /* Print new prompt */
  print_prompt();
} /* execute_command() */

