#include "shell.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "command.h"
#include "single_command.h"

command_t *g_current_command = NULL;
single_command_t *g_current_single_command = NULL;

/*
pid_t shell_pid = 0;         // For ${$}
char * shell_path = NULL;    // For ${SHELL}
*/

int yyparse(void);

/*
 *  Handles zombie processes (child processes that have terminated but
 *  whose exit statuses haven't been collected by the parent process).
 */

extern void zombie(int sig) {
  int ret = wait3(0, 0, NULL);
  while (waitpid(-1, NULL, WNOHANG) > 0) {
    if (isatty(0)) {
      printf("%d exited.\n", ret);
      printf("%d sig value.\n", sig);
    }
  }
} /* zombie() */

/*
 *  Handles the Ctrl-C (SIGINT) signal, allowing the shell to interrupt
 *  the current process and return to the prompt.
 */

extern void ctrl_c(int sig) {
  if (sig == SIGINT) {
    printf("\n");
    print_prompt();
  }
} /* ctrl_c() */

/*
 *  Prints the shell prompt to the terminal.
 */

void print_prompt() {
  if (isatty(0)) {
    printf("myshell>");
  }
  fflush(stdout);
} /* print_prompt() */

/*
 *  Entry point for the program which sets up memory, configures signal
 *  handlers, and turns control over to yyparse for shell parsing.
 */

int main() {
  /*
   * shell_pid = getpid();
   * shell_path = realpath("/proc/self/exe", NULL); // Get the absolute path
   * setenv("SHELL", shell_path, 1); // Set SHELL environment variable
   */

  // Initialize the signal handler for zombie processes (child termination)
  struct sigaction sig_zombie = {0};
  sig_zombie.sa_handler = zombie;
  sigemptyset(&sig_zombie.sa_mask);
  sig_zombie.sa_flags = SA_RESTART;
  int zombie_error = sigaction(SIGCHLD, &sig_zombie, NULL);
  if (zombie_error) {
    perror("sigaction");
    exit(-1);
  }

  // Initialize the signal handler for Ctrl-C (SIGINT)
  struct sigaction sig_ctrlc = {0};
  sig_ctrlc.sa_handler = ctrl_c;
  sigemptyset(&sig_ctrlc.sa_mask);
  sig_ctrlc.sa_flags = SA_RESTART;
  int ctrlc_error = sigaction(SIGINT, &sig_ctrlc, NULL);
  if (ctrlc_error) {
    perror("sigaction");
    exit(-1);
  }

  // Allocate and initialize the current command and single command structures
  g_current_command = (command_t *)malloc(sizeof(command_t));
  g_current_single_command =
      (single_command_t *)malloc(sizeof(single_command_t));

  create_command(g_current_command);
  create_single_command(g_current_single_command);

  print_prompt();
  yyparse();

  // Free memory (uncomment when required)
  // free(shell_path);
  // free_command(g_current_command);
  // free_single_command(g_current_single_command);
} /* main() */
