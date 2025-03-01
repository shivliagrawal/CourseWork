#include <stdio.h>
extern char **environ;

int main() {
  char **s = environ;

  puts("Content-type: text/plain\r\n\r\n");

  for (; *s; s++) {
    printf("%s\n", *s);
  }

  return 0;
}
