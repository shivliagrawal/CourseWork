#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

extern void httprun(int ssock, char *query_string) {
  // Open sub socket for printfing
  FILE *fssock = fdopen(ssock, "r+");
  if (fssock == NULL) {
    perror("fdopen");
  }

  const char *header = "Content-type: text/plain\r\n\r\n";

  write(ssock, header, strlen(header));

  int pid = fork();
  if (pid == -1) {
    perror("date.so fork error");
    return;
  }
  else if (pid == 0) {
    // child process
    close(1);
    dup2(ssock, 1);

    execlp("date", "date", NULL);
    perror("execlp failed");
    exit(-1);
  } else {
    // parent process, child = pid
    printf("parent waiting for child\n");
    wait(NULL);
    printf("parent collected zombie child\n");
  }

}
