
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void filedump(char * p , int len) {
    int i;
    int j=0;
    int c;
    int x;
    if (len % 16 == 0) {
            x = (len/16) - 1;
    } else {
            x = (len/16);
    }

    int z=0;

    while(j <= x) {

            printf("0x%016lX: ", (unsigned long) (z<<4)); // Print address of the beginning of p. You need to print it every 16 bytes
            if(j == x && (len%16) != 0) {
                    int last = len % 16;
                    for (int k=0; k<last; k++) {
                            c = p[k + (16*j)]&0xFF;
                            printf("%02X ", c);
                    }
                    for(int l=0; l<(16 - last); l++) {
                            printf("   ");
                    }
                    printf(" ");
                    for (i=0; i < last; i++) {
                            c = p[i + (16*j)]&0xFF;
                            printf("%c", (c>=32 && c<127)?c:'.');
                    }

                    printf("\n");

            } else if(j != x || (len%16) == 0) {
                 for (i=0; i < 16; i++) {
                            c = p[i+(j*16)]&0xFF; // Get value at [p]. The &0xFF is to make sure you truncate to 8bits or one byte.

                         // Print first byte as hexadecimal
                         printf("%02X ", c);
                 }
                 printf(" ");
                 for (i=0; i < 16; i++) {
                            c = p[i + (16*j)]&0xFF;
                            printf("%c", (c>=32 && c<127)?c:'.');
                 }

                 printf("\n");
            }
	    j++;
	    z++;
    }

}

int
main(int argc, char **argv) {
	FILE*fd = fopen(argv[1], "r");
	if (fd ==NULL) {
		printf("Error opening file \"invalidfile\"\n");
		exit(1);
	}
	fseek(fd, 0L, SEEK_END);  // Go to the end of the file fin
        int fileSize = ftell(fd); // Get current file pointer
        fseek(fd, 0L, SEEK_SET); // Go back to the beginning of the file
	if(argc == 3 && atoi(argv[2]) < fileSize) {
		fileSize = atoi(argv[2]);
	}

	char array[fileSize+1];
	fread(array, 1, fileSize, fd);
	filedump(array, fileSize);


}


