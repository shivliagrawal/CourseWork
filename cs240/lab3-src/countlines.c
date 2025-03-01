
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char ** argv) {
	int lines=0;
        char c;

 	if (argc < 2) {
          printf("Usage: toascii filename\n");
          exit(1);
        }

        // Get file from first argument.
        char * fileName = argv[1];
        FILE * fd = fopen(fileName, "r");
        if (fd == NULL) {
          printf("Could not open file %s\n", fileName);
          exit(1);
        }

        for (c = getc(fd); c != EOF; c = getc(fd))
        if (c == '\n') // Increment count if this character is newline
            lines = lines + 1;

        fclose(fd);
	printf("Program that counts lines.\nTotal lines: %d\n", lines);

	exit(0);
}
