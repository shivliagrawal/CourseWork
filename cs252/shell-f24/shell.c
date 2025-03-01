

#include "shell.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "command.h"
#include "single_command.h"

command_t *g_current_command = NULL;
single_command_t *g_current_single_command = NULL;

int yyparse(void);
int background_running = 0;

/*
 *  Prints shell prompt
 */

void ctrl_c(int sig) {
  if (background_running == 0) {
    print_prompt();
  }
 printf("\n");

  //print_prompt();

}

void zombie(int sig) {
// sigchild
  int pid = waitpid(0, 0, NULL);
  // return immediatly if no child process exited
  // > 0 meaning wait for the child whose process ID is equal to value of pid 
  // create handler that lsitens for sigchild
  // if you recieve from background process, print message
  while(waitpid(-1, NULL, WNOHANG) > 0) {};

}


void print_prompt() {
  if (isatty(0)) {
    printf("myshell>");
  }
  fflush(stdout);
} /* print_prompt() */

/*
 *  This main is simply an entry point for the program which sets up
 *  memory for the rest of the program and the turns control over to
 *  yyparse and never returns
 */

int main() {
  // control_c
  struct sigaction signal_ctrl; // declares var of type sigaction
  signal_ctrl.sa_handler = ctrl_c; // when SIGINT recieved, function executed
  sigemptyset(&signal_ctrl.sa_mask); // ensure that no additional signals blocked when ctrl_c runs
  signal_ctrl.sa_flags = SA_RESTART;

  int error_ctrl = sigaction(SIGINT, &signal_ctrl, NULL);

  if (error_ctrl) {
    perror("sigaction");
    exit(-1);

  }

  // zombie
  struct sigaction signal_zombie;
  signal_zombie.sa_handler = zombie;
  sigemptyset(&signal_zombie.sa_mask);
  signal_zombie.sa_flags = SA_RESTART;

  int error_zombie = sigaction(SIGCHLD, &signal_zombie, NULL);

  if (error_zombie) {
    perror("sigaction");
    exit(-1);

  }

  g_current_command = (command_t *)malloc(sizeof(command_t));
  g_current_single_command =
        (single_command_t *)malloc(sizeof(single_command_t));

  create_command(g_current_command);
  create_single_command(g_current_single_command);

  // handle signalint
  // call control  c

  print_prompt();
  yyparse();
} /* main() */
