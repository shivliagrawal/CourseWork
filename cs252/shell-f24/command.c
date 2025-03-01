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
#include <string.h>

#include "shell.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

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

  printf( "\n\n" );
  printf( "  Output       Input        Error        Background\n" );
  printf( "  ------------ ------------ ------------ ------------\n" );
  printf( "  %-12s %-12s %-12s %-12s\n",
            command->out_file?command->out_file:"default",
            command->in_file?command->in_file:"default",
            command->err_file?command->err_file:"default",
            command->background?"YES":"NO");
  printf( "\n\n" );
} /* print_command() */

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

  // Print contents of Command data structure
  //print_command(command);

  // Add execution here
  // For every single command fork a new process
  // Setup i/o redirection
  // and call exec

  int ret;
  /* dup() duplicates file descripter for stdin/out/err
  and stores in temp variables */
  int tmpin = dup(0);
  // allows shell to restore og streams after command executed
  int tmpout = dup(1);
  int tmperr = dup(2);

  int fdin = 0;
  int fdout = 0;
  int fderr = 0;

  if (command->in_file) {
    // if there is input redirection, open the file
    fdin = open(command->in_file, O_RDONLY);
  }
  else {
    fdin = dup(tmpin); // use default input (stdin)
  }

  /* iterates over every single command */
  for (int i = 0; i < command->num_single_commands; i++) {
     if (strcmp(command->single_commands[i]->executable, "exit") == 0) {
      exit(0);
     }
    /* redirect input */
    dup2(fdin, 0);
    close(fdin); // old file descripter closed, no longer needed

    /* set output for last command */
    /* handles output and error redirection for last command in pipeline */
    if (i == ((command->num_single_commands) - 1)) {
      if (command->out_file) {
        if (command->append_out) {
          fdout = open(command->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        }
        else {
          fdout = open(command->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

      }
      else {
        /* if no output file specified, duplicate og stdout */
        fdout = dup(tmpout);
      }

      if (command->err_file) {
        if (command->append_err) {
          fderr = open(command->err_file, O_WRONLY | O_CREAT | O_APPEND, 0644);

        }
        else {
          fderr = open(command->err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
      }
      else {
        fderr = dup(tmperr);
      }

    }
    else { 
    /* for commands not in the last pipeline
    (all middle stuff) */
      int fdpipe[2];
      pipe(fdpipe);
      fdout = fdpipe[1]; // writing
      fdin = fdpipe[0]; // reading

    }
    /* redirects std output to fdout,
    can be file, pipe, or default stdout */
    dup2(fdout, 1);
    dup2(fderr, 2);
    close(fdout);
    close(fderr);

    ret = fork(); // fork child process
    if (ret == 0) { // child
      /* set last to NULL ptr */
      insert_argument(command->single_commands[i], NULL);
      // edit insert_argiment function to accept NULL
      execvp(command->single_commands[i]->arguments[0], command->single_commands[i]->arguments);


    }
    else if (ret < 0) {
      perror("fork");
      return;

    }


  }
  /* after loop, og stdin/out/err saved
  and stored using dup2() */
  dup2(tmpin, 0);
  dup2(tmpout, 1);
  dup2(tmperr, 2);
  close(tmpin);
  close(tmpout);
  close(tmperr);
  /* waiting for last command to finish
  if not in background */

  if (!command->background) {
    int status;
    waitpid(ret, &status, 0);

  }


  // Clear to prepare for next command
  free_command(command);

  // Print new prompt
  if (isatty(0)) {
    print_prompt();
  }
} /* execute_command() */
