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


/*
 * command.c: Implementation of command execution
 * Updated to remove the 'executable' field and fix double free errors
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

/* Initialize a command_t */
void create_command(command_t *command) {
    command->single_commands = NULL;
    command->num_single_commands = 0;

    command->out_file = NULL;
    command->in_file = NULL;
    command->err_file = NULL;

    command->append_out = false;
    command->append_err = false;
    command->background = false;
}

/* Insert a single command into the command_t */
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
}

/* Free a command_t and its contents */
void free_command(command_t *command) {
    for (int i = 0; i < command->num_single_commands; i++) {
        free_single_command(command->single_commands[i]);
    }

    if (command->single_commands) {
        free(command->single_commands);
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
}

/* Execute the command_t */
void execute_command(command_t *command) {
    // Don't do anything if there are no single commands
    if (command->num_single_commands == 0) {
        print_prompt();
        return;
    }

    // Save default file descriptors
    int tmpin = dup(0);
    int tmpout = dup(1);
    int tmperr = dup(2);

    int fdin;
    if (command->in_file) {
        fdin = open(command->in_file, O_RDONLY);
        if (fdin < 0) {
            perror(command->in_file);
            exit(1);
        }
    } else {
        fdin = dup(tmpin);
    }

    int ret;
    int fdout;
    int i;

    // Iterate over each command
    for (i = 0; i < command->num_single_commands; i++) {
        // Redirect input for first command
        dup2(fdin, 0);
        close(fdin);

        // Set up output redirection
        if (i == command->num_single_commands - 1) {
            // Last command
            if (command->out_file) {
                if (command->append_out) {
                    fdout = open(command->out_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
                } else {
                    fdout = open(command->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                }
                if (fdout < 0) {
                    perror(command->out_file);
                    exit(1);
                }
            } else {
                fdout = dup(tmpout);
            }
        } else {
            // Not last command, create pipe
            int fdpipe[2];
            if (pipe(fdpipe) == -1) {
                perror("pipe");
                exit(1);
            }
            fdout = fdpipe[1];
            fdin = fdpipe[0];
        }

        // Redirect output
        dup2(fdout, 1);
        close(fdout);

        // Set up error redirection
        int fderr;
        if (command->err_file) {
            if (command->append_err) {
                fderr = open(command->err_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
            } else {
                fderr = open(command->err_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            }
            if (fderr < 0) {
                perror(command->err_file);
                exit(1);
            }
        } else {
            fderr = dup(tmperr);
        }

        // Redirect stderr
        dup2(fderr, 2);
        close(fderr);

        // Fork the process
        ret = fork();
        if (ret == 0) {
            // Child process
            single_command_t *cmd = command->single_commands[i];

            // Execute the command using arguments[0] as the executable
            execvp(cmd->arguments[0], cmd->arguments);

            // If execvp returns, there was an error
            perror(cmd->arguments[0]);
            exit(1);
        } else if (ret < 0) {
            // Fork failed
            perror("fork");
            exit(1);
        }

        // Restore stderr in parent
        dup2(tmperr, 2);

        // For the next command, input is from the pipe we set up
        // If there is a next command, fdin is already set to fdpipe[0]
    }

    // Restore default file descriptors
    dup2(tmpin, 0);
    dup2(tmpout, 1);
    dup2(tmperr, 2);
    close(tmpin);
    close(tmpout);
    close(tmperr);

    if (!command->background) {
        // Wait for all child processes
        int status;
        while (wait(&status) > 0);
    }

    // Print new prompt
    print_prompt();
}

